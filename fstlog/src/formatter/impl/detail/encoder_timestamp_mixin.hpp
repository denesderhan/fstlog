//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <chrono> 
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <time.h>
#pragma intrinsic(memcpy)

#include <detail/byte_span.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <detail/utf_conv.hpp>
#include <detail/utf8_len.hpp>
#include <formatter/impl/detail/format_setting_txt.hpp>
#include <formatter/impl/detail/format_str_helper.hpp>
#include <formatter/impl/detail/nano_to_seconds_txt.hpp>
#include <formatter/impl/detail/shift_fill.hpp>
#include <formatter/impl/detail/time_string_cache.hpp>
#include <formatter/impl/detail/tz_format.hpp>
#include <formatter/impl/detail/valid_strftime_string.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
	template<bool use_fill_align, typename L>
    class encoder_timestamp_mixin : public L {
    public:
        using allocator_type = typename L::allocator_type;

        encoder_timestamp_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(encoder_timestamp_mixin(allocator_type{})))
			: encoder_timestamp_mixin(allocator_type{}) {}
		explicit encoder_timestamp_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
            : L(allocator) {}

		encoder_timestamp_mixin(const encoder_timestamp_mixin& other) noexcept(
			noexcept(encoder_timestamp_mixin::get_allocator())
			&& noexcept(encoder_timestamp_mixin(encoder_timestamp_mixin{}, allocator_type{})))
            : encoder_timestamp_mixin(other, other.get_allocator()) {}
		encoder_timestamp_mixin(const encoder_timestamp_mixin& other, allocator_type const& allocator) noexcept(
			noexcept(L(encoder_timestamp_mixin{}, allocator_type{})))
            : L(other, allocator),
			tstr_cache_{ other.tstr_cache_ },
			time_format_{ other.time_format_ },
			second_char_num_{ other.second_char_num_ },
			tzone_{ other.tzone_ } {}

        encoder_timestamp_mixin(encoder_timestamp_mixin&& other) = delete;
        encoder_timestamp_mixin& operator=(const encoder_timestamp_mixin& rhs) = delete;
        encoder_timestamp_mixin& operator=(encoder_timestamp_mixin&& rhs) = delete;
        
        ~encoder_timestamp_mixin() = default;
        
		error_code init_encoder_timestamp(buff_span_const time_format) noexcept {
			if (time_format_.size() != 0) return error_code::double_init;
			const auto begin = time_format.data();
			auto end = begin + time_format.size_bytes();
			
			format_setting_txt format{};
			auto pos = begin;
			
			pos = skip_fill_align(pos, end);
			if constexpr (use_fill_align) {
				auto fill_begin = begin;
				auto align_end = pos;
				if (fill_begin < align_end) format.align = *--align_end;
				memcpy(format.fill_char.data(), fill_begin, static_cast<std::size_t>(align_end - fill_begin));
			}
			const auto pos_0{ pos };
			pos = skip_sign_alt_0(pos, end);
			if (pos != pos_0) {
				// "[sign][#][0] not supported in timestamp formatting (format string)!"
				return error_code::fmt_bad;
			}
			format.width = static_cast<std::uint16_t>(get_width(pos, end));
			int precision = 6;
			get_precision(precision, pos, end);
			if (precision > 9) precision = 9;
			second_char_num_ = precision <= 0 ? 2 : 3 + precision;
			tzone_ = get_zone(pos, end);

			// no strftime string, set default
			if (pos == end) {
				std::string_view strft_str = tzone_ == tz_format::Local ?
					"%Y-%m-%d %H:%M:%S %z"
					: "%Y-%m-%d %H:%M:%S +0000";
				pos = safe_reinterpret_cast<const unsigned char*>(strft_str.data());
				end = safe_reinterpret_cast<const unsigned char*>(strft_str.data() + strft_str.size());
			}
			const auto strftime_str = buff_span_const{ pos, static_cast<std::size_t>(end - pos) };

			if (!detail::valid_strftime_string(strftime_str, tzone_)) return error_code::fmt_bad;
			tstr_cache_.clear();
			auto error = set_time_format(strftime_str, second_char_num_);
			if (error != error_code::none) return error;
			
			// try to format a timestamp without fill-align (cache will store this)
			// the formatted string size does not change, all timestamp size will be the same
			// the formatted string size has to fit into the capacity
			// of the cache's time_string<64>::capacity() (62 bytes)
			std::array<unsigned char, time_string<64>::capacity()> temp{ 0 };
			this->output_span_init(temp);
			encode_timestamp(stamp_type{});
			if (this->has_error()) return error_code::str_long;
			
			// append fill characters if needed
			if constexpr (use_fill_align) {
				const detail::utf8_len formatted_len = detail::utf8_str_trim(temp.data(), this->output_ptr());
				if (formatted_len.char_len < format.width) {
					const std::size_t time_format_bytes = time_format_.size();
					memcpy(temp.data(), time_format_.data(), time_format_bytes);
					// try to add fill characters to the time_format_ but use the formatted stamps char number
					const detail::utf8_len result_len = detail::shift_fill(
						temp, 
						{ time_format_bytes, formatted_len.char_len },
						format);
					// if appending fill chars to time_format succeded
					if (result_len.byte_len != time_format_bytes) {
						std::size_t sec_pos = 0;
						while (sec_pos < result_len.byte_len && *(temp.data() + sec_pos) != 1) sec_pos++;
						if (sec_pos == result_len.byte_len) sec_pos = 255;
						time_format_ = time_string<64>(
							buff_span{ temp.data(),  result_len.byte_len }, 
							static_cast<unsigned char>(sec_pos));
						tstr_cache_.clear();
						this->output_span_init(temp);
						encode_timestamp(stamp_type{});
						if (this->has_error()) return error_code::str_long;
					}
				}
			}
			return error_code::none;
		}

        void encode_timestamp(stamp_type timestamp) noexcept {
			if (timestamp < std::chrono::system_clock::time_point{}) {
				this->set_error(__FILE__, __LINE__, error_code::input_bad);
				return;
			}

			detail::nano_to_seconds_txt sec_f(timestamp, second_char_num_);
			const auto min{ std::chrono::duration_cast<std::chrono::minutes>(sec_f.minutes().time_since_epoch()).count() };
			auto t_str_ptr = tstr_cache_.find(min);
			if (t_str_ptr == nullptr) {
				t_str_ptr = &tstr_cache_.replace_last(
					create_time_string(sec_f.minutes()),
					min);
			}

			const auto s_str_size = t_str_ptr->size();
			if (!this->output_has_space(s_str_size)) {
				this->set_error(__FILE__, __LINE__, error_code::buff_full);
				return;
			}
			const auto out_begin = this->output_ptr();
			const auto in_begin = t_str_ptr->data();
			memcpy( out_begin, in_begin, s_str_size);
			if (t_str_ptr->has_second()) {
				memcpy(
					out_begin + t_str_ptr->second_pos(),
					sec_f.second_str().data(),
					sec_f.second_str().size());
			}
			this->advance_output(s_str_size);
        }

	private:
		error_code set_time_format(buff_span_const strft_str, int second_char_num) noexcept {
			FSTLOG_ASSERT(
				second_char_num == 0
				|| second_char_num == 2
				|| (second_char_num >= 4 && second_char_num <= 12));
			const std::size_t tfstr_size = strft_str.size_bytes();
			if (tfstr_size == 0
				|| tfstr_size > time_string<64>::capacity())
			{
				return error_code::str_long;
			}
			int str_length = static_cast<int>(tfstr_size);
			int sec_pos = 0;
			while (sec_pos < str_length) {
				if (sec_pos < str_length - 1
					&& strft_str[sec_pos] == '%')
				{
					if (strft_str[sec_pos + 1] == 'S') break;
					sec_pos += 2;
				}
				else {
					sec_pos++;
				}
			}

			//no second in string
			if (sec_pos == str_length) {
				time_format_ = time_string<64>(strft_str);
				return error_code::none;
			}

			str_length += second_char_num - 2;
			if (str_length > static_cast<int>(time_string<64>::capacity())) {
				return error_code::str_long;
			}
			std::array<unsigned char, 64> buff{ 0 };
			auto dest_ptr{ buff.data() };
			auto src_ptr{ strft_str.data() };
			memcpy(dest_ptr, src_ptr, sec_pos);
			dest_ptr += sec_pos;
			while (second_char_num-- != 0) {
				*dest_ptr++ = 1;
			}
			const int post_sec_pos{ sec_pos + 2 };
			src_ptr += post_sec_pos;
			memcpy(dest_ptr, src_ptr, strft_str.size_bytes() - post_sec_pos);
			time_format_ = time_string<64>(
				buff_span_const{ buff.data(), static_cast<std::size_t>(str_length) },
				static_cast<unsigned char>(sec_pos));
			return error_code::none;
		}

		time_string<64> create_time_string(stamp_type minutes) noexcept {
			time_t const t{ std::chrono::system_clock::to_time_t(minutes) };
			tm time;
			if (tzone_ == tz_format::UTC) {
#ifdef _WIN32
				/*https://stackoverflow.com/questions/19051762/difference-between-gmtime-r-and-gmtime-s
				thread safety isn't an issue with gmtime and localtime in Microsoft's CRT,
				since these functions'static output areas are already allocated per thread.
				Instead, gmtime_s and localtime_s were added to do the Secure CRT's parameter validation.
				(In other words, they check if their parameters are NULL, in which case they invoke error handling.)*/
				if (gmtime_s(&time, &t) != 0) {
#else
				if (gmtime_r(&t, &time) == NULL) {
#endif
					constexpr auto temp{ "Error in gmtime_.()!" };
					return time_string<64>{byte_span<const char>{temp,
						sizeof("Error in gmtime_.()!") - 1}};
				}
			}
			else {
#ifdef _WIN32
				//setting timezone info (neccessary, zone change on machine, daylight savings time)
				_tzset();
				if (localtime_s(&time, &t) != 0) {
					constexpr auto temp{ "Error in localtime_s()!" };
					return time_string<64>{byte_span<const char>{temp,
						sizeof("Error in localtime_s()!") - 1}};
				}
#else
				//setting timezone info (neccessary, zone change on machine, daylight savings time)
				tzset();
				if (localtime_r(&t, &time) == NULL) {
					constexpr auto temp{ "Error in localtime_r()!" };
					return time_string<64>{byte_span<const char>{temp,
						sizeof("Error in localtime_r()!") - 1}};
				}
#endif
			}
			std::array<char, 64> temp_buff{ 0 };
			static_assert(time_string<64>::capacity() + 1 <= temp_buff.size());
			//strftime writes string with terminating '\0', but returns written size without it!
			//windows CRT throws SEH exception if ptr == null or writeable size == 0!!!
			std::size_t str_size = strftime(
				temp_buff.data(),
				time_string<64>::capacity() + 1,
				time_format_.data(),
				&time);

			if (str_size != 0) {
				unsigned char sec_pos{ 0 };
				if (time_format_.has_second()) {
					while (temp_buff[sec_pos] != 1) {
						sec_pos++;
					};
				}
				else {
					sec_pos = 255;
				}
				return time_string<64>{
					byte_span<const char>{temp_buff.data(), str_size},
						sec_pos};
			}
			else {
				static_assert(sizeof("Formatted timestamp was too long!") - 1
					<= time_string<64>::capacity());
				constexpr auto temp{ "Formatted timestamp was too long!" };
				return time_string<64>{byte_span<const char>{temp,
					sizeof("Formatted timestamp was too long!") - 1 } };
			}
		}

		time_string_cache<7> tstr_cache_;
		time_string<64> time_format_;
		int second_char_num_{ 9 };
		tz_format tzone_{ tz_format::Local };
    };
}
