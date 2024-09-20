//Copyright © 2022, Dénes Derhán.
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

#include <detail/buffer_operation_result.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <formatter/impl/detail/nano_to_seconds_txt.hpp>
#include <formatter/impl/detail/time_string_cache.hpp>
#include <formatter/impl/detail/tz_format.hpp>
#include <formatter/impl/detail/valid_strftime_string.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
	class time_to_str_converter
	{
	public:
		const char* init_time_to_str_converter(
			buff_span_const time_format,
			int second_precision = 6,
			tz_format tzone = tz_format::Local) noexcept
		{
			if (!detail::valid_strftime_string(time_format, tzone)) return "Invalid strftime string!";
			tstr_cache_.clear();
			if (second_precision > 9 || second_precision < 0) second_precision = 6;
			second_char_num_ = second_precision == 0 ? 2 : 3 + second_precision;
			tzone_ = tzone;
			return set_time_format(time_format, second_char_num_);
		}

		detail::buffer_operation_result<unsigned char> timestamp_to_chars(
			stamp_type timestamp, 
			unsigned char* buffer_ptr, 
			const unsigned char* buffer_end) noexcept 
		{
			detail::nano_to_seconds_txt sec_f(timestamp, second_char_num_);
			const auto min{ std::chrono::duration_cast<std::chrono::minutes>(sec_f.minutes().time_since_epoch()).count() };
			auto t_str_ptr = tstr_cache_.find(min);
			if (t_str_ptr == nullptr) {
				t_str_ptr = &tstr_cache_.replace_last(
					create_time_string(sec_f.minutes()),
					min);
			}

			auto s_str_size = t_str_ptr->size();
			if (buffer_ptr + s_str_size <= buffer_end) {
				memcpy(
					buffer_ptr,
					t_str_ptr->data(),
					s_str_size
				);
				if(t_str_ptr->has_second()) {
					memcpy(
						buffer_ptr + t_str_ptr->second_pos(),
						sec_f.second_str().data(),
						sec_f.second_str().size()
					);
				}
				buffer_ptr += s_str_size;
				return detail::buffer_operation_result<unsigned char>{ buffer_ptr, error_code::none };
			}
			else {
				return detail::buffer_operation_result<unsigned char>{ buffer_ptr, error_code::no_space_in_buffer };
			}
		}

	private:
		const char* set_time_format(buff_span_const strft_str, int second_char_num) noexcept {
			FSTLOG_ASSERT(
				second_char_num == 0
				|| second_char_num == 2
				|| (second_char_num >= 4 && second_char_num <= 12));
			const std::size_t tfstr_size = strft_str.size_bytes();
			if (tfstr_size == 0
				|| tfstr_size > time_string<64>::capacity())
				return "Strftime string too long or empty!";
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
				return nullptr;
			}

			str_length += second_char_num - 2;
			if (str_length > static_cast<int>(time_string<64>::capacity()))
				return "Strftime string too long!";
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
			return nullptr;
		}

		time_string<64> create_time_string(stamp_type minutes) noexcept {
			if (minutes < std::chrono::system_clock::time_point{ std::chrono::seconds{0} }) {
				constexpr auto temp{ "Pre epoch timestamp!" };
				return time_string<64>{byte_span<const char>{temp,
					sizeof("Pre epoch timestamp!") - 1 } };
			}
			time_t const t{	std::chrono::system_clock::to_time_t(minutes) };
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
				&time
			);
			
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
