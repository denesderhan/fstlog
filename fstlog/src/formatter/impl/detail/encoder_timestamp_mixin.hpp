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
#pragma intrinsic(memcpy, memset)

#include <detail/unaligned_span.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <detail/utf_conv.hpp>
#include <detail/utf8_len.hpp>
#include <formatter/impl/detail/format_setting_txt.hpp>
#include <formatter/impl/detail/format_str_helper.hpp>
#include <formatter/impl/detail/local_utc_offset.hpp>
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
			time_string_cache_{ other.time_string_cache_ },
			time_format_{ other.time_format_ },
			tzone_{ other.tzone_ },
			formatted_length_{ other.formatted_length_ },
			second_precision_{ other.second_precision_ },
			second_pos_{ other.second_pos_ } {}

		encoder_timestamp_mixin(encoder_timestamp_mixin&& other) = delete;
		encoder_timestamp_mixin& operator=(const encoder_timestamp_mixin& rhs) = delete;
		encoder_timestamp_mixin& operator=(encoder_timestamp_mixin&& rhs) = delete;

		~encoder_timestamp_mixin() = default;

		error_code init_encoder_timestamp(byte_span_const time_format) noexcept {
			if (!time_format_.empty()) {
				return error_code::double_init;
			}

			format_setting_txt format{};
			auto error = decompose_format(time_format, format);
			if (error != error_code::none) return error;
			// try to set time_format_
			error = set_time_format(time_format);
			if (error != error_code::none) return error;
						
			// we append the fill characters to the time_format_ string 
			// this is safe because the formatted length is constant
			error = apply_fill_align(format);
			if (error != error_code::none) return error;
			
			prepare_seconds_placeholder();

			return error_code::none;
		}

		void encode_timestamp(stamp_type timestamp) noexcept {
			if (timestamp < std::chrono::system_clock::time_point{}) {
				this->set_error(__FILE__, __LINE__, error_code::input_bad);
				return;
			}
			if (!this->output_has_space(formatted_length_)) {
				this->set_error(__FILE__, __LINE__, error_code::buff_full);
				return;
			}

			const auto minutes{ std::chrono::floor<std::chrono::minutes>(timestamp) };
			const auto nanoseconds{ std::chrono::duration_cast<std::chrono::nanoseconds>(
				timestamp - minutes) };
			
			const auto key{ minutes.time_since_epoch().count() };
			// search the minute in the cache
			auto time_string{ time_string_cache_.find(key) };
			// if not found create and store
			if (time_string.empty()) {
				time_string = create_time_string(minutes);
				FSTLOG_ASSERT(time_string.size() == formatted_length_);
				time_string_cache_.replace_oldest(time_string, key);
			}

			const auto out_begin = this->output_ptr();
			memcpy(out_begin, time_string.data(), formatted_length_);
			// replacing the second placeholder with the second string
			write_second(nanoseconds, out_begin);
			
			this->advance_output(formatted_length_);
		}

	private:

		// decompose fill-align, precision and time_format string
		error_code decompose_format(
			byte_span_const& time_format,
			format_setting_txt& format) noexcept
		{
			const auto begin = time_format.data();
			const auto end = begin + time_format.size_bytes();
			auto pos = begin;

			pos = skip_fill_align(pos, end);
			if constexpr (use_fill_align) {
				auto fill_begin = begin;
				auto align_end = pos;
				if (fill_begin < align_end) format.align = *--align_end;
				memcpy(format.fill_char.data(), fill_begin, static_cast<std::size_t>(align_end - fill_begin));
			}

			auto pos0 = pos;
			// sign, alt '0' is ignored
			pos = skip_sign_alt_0(pos, end);
			if (pos != pos0) {
				return error_code::fmt_bad;
			}

			format.width = static_cast<std::uint16_t>(get_width(pos, end));
			// precision is the precision of seconds (precision 0: "30" precision 2 : "30.44")
			int precision = 6;
			get_precision(precision, pos, end);
			if (precision > 9) precision = 9;
			second_precision_ = static_cast<unsigned char>(precision);
			
			time_format = byte_span_const(pos, static_cast<std::size_t>(end - pos));
			return error_code::none;
		}

		// setting the time format string
		error_code set_time_format(byte_span_const format_string) noexcept {
			auto pos = format_string.data();
			auto end = pos + format_string.size_bytes();
			tzone_ = get_zone(pos, end);
			
			// no strftime string, set default
			if (pos == end) {
				if (tzone_ == tz_format::Local) {
					time_format_ = small_string<64>("%Y-%m-%d %H:%M:%S %z");
				}
				else {
					time_format_ = small_string<64>("%Y-%m-%d %H:%M:%S +0000");
				}
			}
			else {
				std::size_t str_len = static_cast<std::size_t>(end - pos);
				if(str_len > time_format_.capacity()) return error_code::str_long;
				time_format_ = small_string<64>(
					safe_reinterpret_cast<const char*>(pos),
					str_len);
			}
			if (!detail::valid_strftime_string(
					byte_span_const(
						safe_reinterpret_cast<const unsigned char*>(time_format_.data()), 
						time_format_.size()), tzone_))
			{
				return error_code::fmt_bad;
			}

			auto error = add_second_placeholder();
			if (error != error_code::none) return error;

			// try to format a timestamp ( without replacing the seconds placeholder )
			auto stamp_str = create_time_string(stamp_type{});
			// if empty, there was not enough space (string size can grow do to formatting)
			if (stamp_str.empty()) return error_code::str_long;
			// the formatted string size will be a constant ( no variable length strftime format specifiers are allowed)
			formatted_length_ = static_cast<unsigned char>(stamp_str.size());
									
			return error_code::none;
		}

		// compute second offset and fill it with '0'-s
		void prepare_seconds_placeholder() noexcept {
			auto stamp_str = create_time_string(stamp_type{});
			std::size_t sec_pos = 0;
			while (sec_pos < stamp_str.size() && *(stamp_str.data() + sec_pos) != 1) sec_pos++;
			if (sec_pos == stamp_str.size()) sec_pos = 255;
			second_pos_ = static_cast<unsigned char>(sec_pos);
			
			// replace 0x1-s with '0'-s
			std::array<char, 64> temp{ 0 };
			memcpy(temp.data(), time_format_.data(), time_format_.size());
			for (auto& c : temp) if (c == 1) c = '0';
			time_format_ = small_string<64>(temp.data(), time_format_.size());
		}

		// pre formatting the format string with fill-align
		error_code apply_fill_align(format_setting_txt format) noexcept {
			if constexpr (use_fill_align) {
				// compute string lengths for the formatted timestamp
				auto stamp_str = create_time_string(stamp_type{});
				const detail::utf8_len formatted_len =
					detail::utf8_str_trim(stamp_str.data(), stamp_str.data() + stamp_str.size());

				// do not truncate
				if (formatted_len.char_len < format.width) {
					const std::size_t time_format_bytes = time_format_.size();
					std::array<unsigned char, small_string<64>::capacity()> temp{};
					memcpy(temp.data(), time_format_.data(), time_format_bytes);
					// try to add fill characters to the time_format_ 
					// but use the formatted character number instead of the format string character number
					const detail::utf8_len result_len = detail::shift_fill(
						temp,
						{ time_format_bytes, formatted_len.char_len },
						format);
					// replace time_format_ if appending succeded
					if (result_len.byte_len > time_format_bytes) {
						std::size_t new_length = formatted_length_ + (result_len.byte_len - time_format_bytes);
						if (new_length > time_format_.capacity()) return error_code::str_long;
						formatted_length_ = static_cast<unsigned char>(new_length);
						time_format_ = small_string<64>(
							safe_reinterpret_cast<const char*>(temp.data()), result_len.byte_len);
					}
				}
			}
			return error_code::none;
		}

		// in time_format_ replaces %S with 0x1-s
		error_code add_second_placeholder() noexcept {
			auto str_len = time_format_.size();
			std::size_t sec_pos = 0;
			while (sec_pos + 1 < str_len
				&& !(*(time_format_.data() + sec_pos) == '%'
					&& *(time_format_.data() + sec_pos + 1) == 'S'))
			{
				sec_pos++;
			}

			// if format string has %S 
			if (sec_pos + 1 < str_len) {
				if (second_precision_ != 0) {
					// %S (length 2) -> "ss" or "ss.fffff"
					// length grow of string: 1('.') + fraqtional digits
					str_len += 1ULL + second_precision_;
				}
				if (str_len > static_cast<int>(small_string<64>::capacity())) {
					return error_code::str_long;
				}
				std::array<char, 64> buff{ 0 };
				auto dest_ptr{ buff.data() };
				auto src_ptr{ time_format_.data() };
				memcpy(dest_ptr, src_ptr, sec_pos);
				dest_ptr += sec_pos;
				std::size_t second_char_num = second_precision_ == 0 ? 2 : 3ULL + second_precision_;
				memset(dest_ptr, 1, second_char_num);
				dest_ptr += second_char_num;
				const auto post_sec_pos{ sec_pos + 2 };
				src_ptr += post_sec_pos;
				memcpy(dest_ptr, src_ptr, time_format_.size() - post_sec_pos);
				// replace time_format_
				time_format_ = small_string<64>(buff.data(), str_len);
			}
			return error_code::none;
		}

		// returns the formatted time local/UTC according to tzone_
		// or empty string on error/not enough space
		// does not fill the seconds place
		small_string<64> create_time_string(stamp_type minutes) noexcept {
			time_t const t{ std::chrono::system_clock::to_time_t(minutes) };
			tm time;		
#ifdef _WIN32
			errno_t error{};
			if (tzone_ == tz_format::UTC) error = gmtime_s(&time, &t);
			else error = localtime_s(&time, &t);
			if (error != 0) return small_string<64>{};
#else		
			tm* error{};
			if (tzone_ == tz_format::UTC) error = gmtime_r(&t, &time);
			else error = localtime_r(&t, &time);
			if (error == nullptr) return small_string<64>{};
#endif
			return format_time(time);
		}

		small_string<64> format_time(std::tm const& time) {
			if (time_format_.empty()) return small_string<64>();
			std::array<char, 160> buff{ 0 };
			// proving that the length can not grow out of the buffer
			static_assert(
				(time_format_.capacity() / 2) * 5 // worst case for length grow (%z 5)
				+ 1 // upper bound in case of odd length
				+ 1 // 0 at end (to be safe)
				< buff.size(), "buff small");
			const auto form_str_len = time_format_.size();
			std::size_t form_pos = 0;
			std::size_t pos = 0;
			while (form_pos < form_str_len - 1) {
				char c0 = *(time_format_.data() + form_pos++);
				// not a format specifier
				if (c0 != '%') {
					buff[pos++] = c0;
					continue;
				}
				
				int num{ 0 };
				const char c1 = *(time_format_.data() + form_pos++);
				// %% (%)
				if (c1 == '%') {
					buff[pos++] = '%';
				}
				// %Y (year 2025)
				else if (c1 == 'Y') {
					std::size_t year = 1900;
					year += time.tm_year;
					buff[pos + 3] = '0' + year % 10; year /= 10;
					buff[pos + 2] = '0' + year % 10; year /= 10;
					buff[pos + 1] = '0' + year % 10; year /= 10;
					buff[pos] = '0' + year % 10;
					pos += 4;
				}
				// %m (month 01-12)
				else if (c1 == 'm') {
					num = time.tm_mon + 1;
					c0 = 0;
				}
				// %d (day 01-31)
				else if (c1 == 'd') {
					num = time.tm_mday;
					c0 = 0;
				}
				// %H (hour 00-23)
				else if (c1 == 'H') {
					num = time.tm_hour;
					c0 = 0;
				}
				// %M (min 00-59)
				else if (c1 == 'M') {
					num = time.tm_min;
					c0 = 0;
				}
				// %z (UTC offset "+0000")
				else if (c1 == 'z') {
					if (time.tm_isdst > 0) {
						memcpy(&buff[pos], utc_offset_.daylight.data(), 5);
					}
					else {
						memcpy(&buff[pos], utc_offset_.standard.data(), 5);
					}
					pos += 5;
				}
				// %y (year 25)
				else if (c1 == 'y' && time.tm_year <= (std::numeric_limits<int>::max)() - 1900) {
					num = time.tm_year + 1900;
					c0 = 0;
				}
				// %a (day abbreviated)
				else if (c1 == 'a' && time.tm_wday >= 0 && time.tm_wday <= 6) {
					const auto days = "SunMonTueWedThuFriSat";
					memcpy(&buff[pos], days + (time.tm_wday * 3), 3);
					pos += 3;
				}
				// unsupported "%?"
				else {
					buff[pos] = '%';
					buff[pos + 1] = c1;
					pos += 2;
				}
				// write number
				if (c0 == 0 && num >= 0) {
					buff[pos + 1] = '0' + num % 10;
					num /= 10;
					buff[pos] = '0' + num % 10;
					pos += 2;
				}
			}
			// if there is a 1 char remainder at the end of form_str
			if (form_pos < form_str_len) {
				buff[pos++] = *(time_format_.data() + form_pos++);
			}
			FSTLOG_ASSERT(pos < buff.size());
			// if not enough space in small_string<64>
			if (pos > small_string<64>::capacity()) {
				return small_string<64>{};
			}
			else {
				return small_string<64>(buff.data(), pos);
			}
		}

		inline void write_second(
			std::chrono::nanoseconds nanoseconds, 
			unsigned char* timestring_begin)
		{
			if (second_pos_ == 255) return; // no second in format string
			
			auto nano_ticks = nanoseconds.count();
			FSTLOG_ASSERT(nano_ticks < 60'000'000'000LL);

			const long long pow10[10] { 1, 10, 100, 1'000, 10'000, 100'000,
				 1'000'000, 10'000'000, 100'000'000, 1'000'000'000 };

			const int digits_cut = 9 - second_precision_;
			nano_ticks /= pow10[digits_cut];

			const char digits2[]{
				"0001020304050607080910111213141516171819"
				"2021222324252627282930313233343536373839"
				"4041424344454647484950515253545556575859"
				"6061626364656667686970717273747576777879"
				"8081828384858687888990919293949596979899" };
			const auto second_begin = timestring_begin + second_pos_;
			unsigned char* pos = second_precision_ == 0 ? 
				second_begin + 2
				: second_begin + 3 + second_precision_;
			// converting digits, two at a time
			while (nano_ticks != 0) {
				pos -= 2;
				const auto next_nano = nano_ticks / 100;
				const auto fraq = nano_ticks - next_nano * 100;
				const int digit_ind = static_cast<int>(fraq * 2);
				*pos = digits2[digit_ind];
				*(pos + 1) = digits2[digit_ind + 1];
				nano_ticks = next_nano;
			}

			// inserting decimal point
			if (second_precision_ != 0) {
				*second_begin = *(second_begin + 1);
				*(second_begin + 1) = *(second_begin + 2);
				*(second_begin + 2) = '.';
			}
		}

		time_string_cache<7> time_string_cache_;
		small_string<64> time_format_;
		detail::utc_offset utc_offset_{ detail::get_utc_offset() };
		tz_format tzone_{ tz_format::Local };
		unsigned char formatted_length_{ 0 };
		unsigned char second_precision_{ 6 };
		unsigned char second_pos_{ 255 };
    };
}
