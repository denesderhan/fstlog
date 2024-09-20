//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>

#include <detail/byte_span.hpp>
#include <fstlog/detail/constants.hpp>
#include <formatter/impl/detail/tz_format.hpp>
#include <detail/utf8_helper.hpp>

namespace fstlog {
	namespace detail{
		
		template<std::size_t N>
		inline constexpr bool arr_contains_char(
			char c,
			const char(&str)[N] ) noexcept
		{
			for (int i = 0; i < static_cast<int>(N); i++) {
				if (str[i] == c) return true;
			}
			return false;
		}

		//Validate time string for strftime call.
		inline constexpr bool valid_strftime_string(
			buff_span_const str, 
			tz_format tz) noexcept 
		{
			constexpr char conv_specs[]{ 
				'%', 'a', 'A', 'b', 'B', 'C', 'd', 'D',
				'e', 'F', 'g', 'G', 'h', 'H', 'I', 'j',
				'm', 'M', 'n', 'p', 'R', 't', 'u', 'U',
				'V', 'w', 'W', 'x',	'y', 'Y'
			};
			constexpr char conv_specs_O[]{
				'y', 'm', 'U', 'W', 'V', 'd', 
				'e', 'w', 'u', 'H', 'I', 'M'
			};
			constexpr char conv_specs_E[]{
				'C', 'x', 'y', 'Y'
			};
			
			const std::size_t str_len = str.size_bytes();
			if (str_len == 0) return true;

			const auto str_end{ str.data() + str_len };
			std::size_t pos = 0;
			std::size_t num_of_sec{ 0 };
			while (pos < str_len) {
				const auto bytes{ valid_utf8(&str[pos], str_end) };
				//invalid utf8
				if (bytes == 0) return false;
				if (!printable_utf8(&str[pos], bytes)) return false;
				
				if (str[pos] == '%') {
					if (pos + 1 == str_len) return false;
					if (str[pos + 1] == 'S') {
						++num_of_sec;
						pos += 2;
					}
					else if (str[pos + 1] == 'z' || str[pos + 1] == 'Z') {
						if (tz == tz_format::UTC) return false;
						pos += 2;
					}
					else if (arr_contains_char(str[pos + 1], conv_specs)) 
					{
						pos += 2;
					}
					else {
						if (pos + 2 == str_len) return false;
						if (str[pos + 1] == 'O' && 
							arr_contains_char(str[pos + 2], conv_specs_O)) 
						{
								pos += 3;
						}
						else if (str[pos + 1] == 'E' &&
							arr_contains_char(str[pos + 2], conv_specs_E))
						{
							pos += 3;
						}
						else return false;
					}
				}
				else{
					pos += bytes;
				}
			}
			return num_of_sec <= 1;
		}
	}
}
