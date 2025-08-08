//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstddef>
#include <cstring>
#pragma intrinsic(memcpy, memset, memmove)

#include <detail/safe_reinterpret_cast.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <formatter/impl/detail/format_setting_txt.hpp>
#include <detail/unaligned_span.hpp>
#include <detail/utf8_len.hpp>

namespace fstlog::detail {
	namespace {
		inline void fill_with_pattern(
			byte_span to_fill,
			std::array<unsigned char, 4> pattern,
			unsigned char pattern_bytes) noexcept
		{
			FSTLOG_ASSERT(to_fill.data_bytes() != nullptr);
			FSTLOG_ASSERT(pattern_bytes > 0 && pattern_bytes <= 4 
				&& to_fill.size_bytes() % pattern_bytes == 0);
			
			if (to_fill.empty()) return;

			if (pattern_bytes == 1) {
				memset(to_fill.data_bytes(), pattern[0], to_fill.size_bytes());
			}
			else {
				auto pos = to_fill.data_bytes();
				auto end = to_fill.data_bytes() + to_fill.size_bytes();
				while (pos < end) {
					memcpy(pos, pattern.data(), pattern_bytes);
					pos += pattern_bytes;
				}
			}
		}
	}

    inline utf8_len shift_fill(
        byte_span buffer,
        utf8_len str_len,
        format_setting_txt fmt) noexcept
    {
        FSTLOG_ASSERT(
			buffer.data_bytes() != nullptr
			&& str_len.byte_len <= buffer.size_bytes());
		FSTLOG_ASSERT(fmt.fill_char[0] != 0);

		// no space to shift
		if (str_len.char_len >= fmt.width) return str_len;

		unsigned char fill_char_bytes = 1;
		while (fill_char_bytes < 4 && fmt.fill_char[fill_char_bytes] != 0) fill_char_bytes++;
		
		const std::size_t length_grow_in_chars = static_cast<std::size_t>(fmt.width) - str_len.char_len;
		const std::size_t length_grow_in_bytes = length_grow_in_chars * fill_char_bytes;
        
		// not enough space in buffer
		if (length_grow_in_bytes > buffer.size_bytes() - str_len.byte_len) return str_len;
        
        if (fmt.align == '>') {
            const auto dest_ptr{ buffer.data_bytes() + length_grow_in_bytes};
            memmove(dest_ptr, buffer.data_bytes(), str_len.byte_len);
			fill_with_pattern(
				{ buffer.data_bytes(), length_grow_in_bytes },
				fmt.fill_char, 
				fill_char_bytes);
        }
        else if (fmt.align == '^') {
			const auto left_fill_bytes = (length_grow_in_chars / 2) * fill_char_bytes;
			const auto right_fill_bytes = length_grow_in_bytes - left_fill_bytes;
            const auto shifted_str_begin{ buffer.data_bytes() + left_fill_bytes };
            memmove(shifted_str_begin, buffer.data_bytes(), str_len.byte_len);
			fill_with_pattern(
				{ buffer.data_bytes(), left_fill_bytes },
				fmt.fill_char, 
				fill_char_bytes);
			fill_with_pattern(
				{ shifted_str_begin + str_len.byte_len, right_fill_bytes },
				fmt.fill_char, 
				fill_char_bytes);
		}
        else {
            //align '<' or any other
			fill_with_pattern(
				{ buffer.data_bytes() + str_len.byte_len, length_grow_in_bytes},
				fmt.fill_char,
				fill_char_bytes);
        }
		return {str_len.byte_len + length_grow_in_bytes , fmt.width };
    }
}
