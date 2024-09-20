//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstring>
#pragma intrinsic(memset, memmove)

#include <detail/buffer_operation_result.hpp>
#include <detail/error_code.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <detail/utf8_helper.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
	namespace {
		inline void fill_with_pattern(
			void* dest_begin,
			std::size_t dest_bytes,
			const void* pattern_begin,
			int pattern_bytes) noexcept
		{
			FSTLOG_ASSERT(dest_begin != nullptr && pattern_begin != nullptr);
			FSTLOG_ASSERT(pattern_bytes > 0 && dest_bytes % pattern_bytes == 0);
			
			auto pos_ptr = safe_reinterpret_cast<unsigned char*>(dest_begin);
			const auto pattern_beg = safe_reinterpret_cast<const unsigned char*>(pattern_begin);
			if (pattern_bytes == 1) {
				const auto p1{ *pattern_beg };
				memset(dest_begin, p1, dest_bytes);
			}
			else if (pattern_bytes == 2) {
				const auto p1{ *pattern_beg };
				const auto p2{ *(pattern_beg + 1) };
				const auto end{ pos_ptr + dest_bytes };
				while (pos_ptr < end) {
					*pos_ptr = p1;
					*(pos_ptr + 1) = p2;
					pos_ptr += 2;
				}
			}
			else {
				auto pattern_ptr{ pattern_beg };
				const auto pattern_end{ pattern_beg + pattern_bytes };
				const auto end{ pos_ptr + dest_bytes };
				while (pos_ptr < end) {
					*pos_ptr++ = *pattern_ptr++;
					if (pattern_ptr >= pattern_end) pattern_ptr = pattern_beg;
				}
			}
		}
	}

    inline detail::buffer_operation_result<unsigned char> shift_fill(
        unsigned char* begin,
        const unsigned char* end,
        std::size_t str_bytes,
        std::size_t str_chars,
        unsigned int width,
        unsigned int align,
        const unsigned char* fill_char) noexcept
    {
        FSTLOG_ASSERT(
			begin != nullptr 
			&& end >= begin 
			&& str_bytes <= static_cast<std::size_t>(end - begin));
		if (width > str_chars) {
			unsigned char first_fill_char{ *fill_char };
			auto fill_ptr{ fill_char };
			int pattern_size{ utf8_bytes(first_fill_char) };
			if (pattern_size == 0) {
				first_fill_char = ' ';
				fill_ptr = &first_fill_char;
				pattern_size = 1;
			}
			const std::size_t length_grow_in_bytes =
				(static_cast<std::size_t>(width) - str_chars) 
				* static_cast<std::size_t>(pattern_size);
            
            if (length_grow_in_bytes <= static_cast<std::size_t>(end - begin) - str_bytes)
            {
                if (align == '>') {
                    const auto dest_ptr{ begin + length_grow_in_bytes };
                    memmove(dest_ptr, begin, str_bytes);
                    fill_with_pattern(begin, length_grow_in_bytes, fill_ptr, pattern_size);
                }
                else if (align == '^') {
                    const auto shift_center = 
						((length_grow_in_bytes / pattern_size) / 2) * pattern_size;
                    const auto shifted_str_begin{ begin + shift_center };
                    memmove(shifted_str_begin, begin, str_bytes);
                    fill_with_pattern(begin, shift_center, fill_ptr, pattern_size);
					// shift_center <= length_grow_in_bytes
					// && (length_grow_in_bytes - shift_center) % pattern_size == 0
					fill_with_pattern(
						shifted_str_begin + str_bytes, 
						length_grow_in_bytes - shift_center,
						fill_ptr,
						pattern_size);
				}
                else {
                    //align '<' or '0' ("default")
                    fill_with_pattern(begin + str_bytes, length_grow_in_bytes, fill_ptr, pattern_size);
                }
				return detail::buffer_operation_result<unsigned char>{
					begin + str_bytes + length_grow_in_bytes, 
					error_code::none};
            }
            else {
                return detail::buffer_operation_result<unsigned char>{begin + str_bytes, error_code::no_space_in_buffer};
            }
        }
        else {
            return detail::buffer_operation_result<unsigned char>{begin + str_bytes, error_code::none};
        }
    }
}
