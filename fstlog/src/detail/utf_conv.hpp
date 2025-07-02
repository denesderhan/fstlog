//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#pragma intrinsic(memcpy)

#include <detail/buffer_operation_result.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/rm_cvref_t.hpp>

namespace fstlog {
	namespace detail {
		namespace utf_const {
			inline constexpr std::uint32_t replacement_char = 0xFFFD;
			inline constexpr std::uint32_t max_code_p = 0x10FFFF;
			inline constexpr std::uint32_t surrogate_start = 0xD800;
			inline constexpr std::uint32_t surrogate_end = 0xDFFF;
			inline constexpr std::uint32_t min_2_byte_code_p = 0x80;
			inline constexpr std::uint32_t min_3_byte_code_p = 0x800;
			inline constexpr std::uint32_t min_4_byte_code_p = 0x10000;

			// Masks and lead patterns for continuation bytes
			inline constexpr std::uint32_t cont_prefix_mask = 0xC0; // 0b1100'0000
			inline constexpr std::uint32_t cont_prefix = 0x80; // 0b1000'0000
			inline constexpr std::uint32_t cont_data_mask = 0x3F; // 0b0011'1111
		}

		template <typename T>
		inline bool valid_utf32_char(T utf32_char) noexcept {
			static_assert(std::is_integral_v<T>, "Invalid type!");
			// valid utf32_char: 0 - 0xFFFF'FFFF
			if constexpr ((std::numeric_limits<T>::min)() < 0) {
				if (utf32_char < 0) return false;
			}
			if constexpr ((std::numeric_limits<T>::max)() > 0xFFFF'FFFFU) {
				if (utf32_char > 0xFFFF'FFFFU) return false;
			}
			return true;
		}

		template <typename T>
		inline bool valid_utf16_char(T utf16_char) noexcept {
			static_assert(std::is_integral_v<T>, "Invalid type!");
			// valid utf16_char: 0 - 65'535
			if constexpr ((std::numeric_limits<T>::min)() < 0) {
				if (utf16_char < 0) return false;
			}
			if constexpr ((std::numeric_limits<T>::max)() > 65'535) {
				if (utf16_char > 65'535) return false;
			}
			return true;
		}

		template <typename T>
		inline bool valid_utf8_char(T utf8_char) noexcept {
			static_assert(std::is_integral_v<T>, "Invalid type!");
			// valid utf8_char: 0 - 255
			if constexpr ((std::numeric_limits<T>::min)() < 0) {
				if (utf8_char < 0) return false;
			}
			if constexpr ((std::numeric_limits<T>::max)() > 255) {
				if (utf8_char > 255) return false;
			}
			return true;
		}

		// Semantic check for surrogates, and out-of-range values.
		constexpr bool valid_utf_code_point(std::uint32_t code_point) noexcept {
			return ( code_point < utf_const::surrogate_start  // under surrogate range
					|| (code_point > utf_const::surrogate_end // between surrogate end and max
						&& code_point <= utf_const::max_code_p));
		}

		// determine if a valid utf code point is in the safe whitelist
		constexpr bool safe_utf_code_point(std::uint32_t code_point) noexcept {
			FSTLOG_ASSERT(
				code_point < utf_const::surrogate_start
				|| (code_point > utf_const::surrogate_end && code_point <= utf_const::max_code_p));

			// Safe ranges [inclusive start, exclusive end).
			// All ranges between these safe ranges are considered unsafe
			// Values must be sorted and unique!
			alignas(constants::cache_ls_nosharing) constexpr std::array<std::uint16_t, 18> sparse_lut{
				0x0020, 0x007F,   // Printable ASCII
				0x00A1, 0x02B0,   // Latin-1 Supplement, extended, IPA
				0x0370, 0x0700,   // Greek, Cyrillic, Armenian, Hebrew, Arabic
				0x0900, 0x09FF,   // Devanagari, Bengali (India)
				0x0E00, 0x1100,   // Thai, Lao, Tibetan, Myanmar, Georgian
				0x2070, 0x20C0,   // Superscript, Subscript, Currency
				0x3000, 0x3100,   // CJK symbols/punktuation, Hiragana, Katakana (japanese)
				0x3300, 0xA000,   // CJK (chinese)
				0xAC00, 0xD7B0    // Hangul Syllables (korean)
			};
			constexpr std::uint32_t range_end = sparse_lut.back();

			// Early reject outside supported range
			if (code_point >= range_end) return false;
			
			bool is_safe = false; // we start with an unsafe range [0, 0x20) 
			for (auto boundary : sparse_lut) {
				if (code_point < boundary) break; // we found the range that the code_point is in
				is_safe = !is_safe; // boundary marks always a change, we flip is_safe when stepping over 
			}
			return is_safe;
		}

		static_assert(!safe_utf_code_point(utf_const::replacement_char), "Replacement char must be unsafe!");

		inline detail::buffer_operation_result<unsigned char> encode_escaped(
			std::uint32_t code_point,
			unsigned char* pos,
			const unsigned char* end) noexcept
		{
			FSTLOG_ASSERT(pos != nullptr && pos <= end);
			FSTLOG_ASSERT(code_point <= utf_const::max_code_p);
			int str_length = code_point <= 0xffff ? 6 : 10;
			if (end - pos < str_length) {
				return detail::buffer_operation_result<unsigned char>{ pos, error_code::buff_full };
			}

			constexpr std::array<unsigned char, 16> hex_digits{
				'0', '1', '2', '3', '4', '5', '6', '7',
				'8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

			if (str_length == 6) {
				*pos = '\\';
				*(pos + 1) = 'u';
				*(pos + 2) = hex_digits[code_point >> 12];
				*(pos + 3) = hex_digits[(code_point >> 8) & 0x0F];
				*(pos + 4) = hex_digits[(code_point >> 4) & 0x0F];
				*(pos + 5) = hex_digits[code_point & 0x0F];
			}
			else {
				*pos = '\\';
				*(pos + 1) = 'U';
				*(pos + 2) = '0';
				*(pos + 3) = '0';
				*(pos + 4) = hex_digits[(code_point >> 20) & 0x0F]; // must mask for release build and bad code point 
				*(pos + 5) = hex_digits[(code_point >> 16) & 0x0F];
				*(pos + 6) = hex_digits[(code_point >> 12) & 0x0F];
				*(pos + 7) = hex_digits[(code_point >> 8) & 0x0F];
				*(pos + 8) = hex_digits[(code_point >> 4) & 0x0F];
				*(pos + 9) = hex_digits[code_point & 0x0F];
			}

			pos += str_length;
			return detail::buffer_operation_result<unsigned char>{ pos, error_code::none };
		}

		// Decodes a single UTF-8 code point from the buffer.
		// Advances 'pos' by the number of bytes consumed.
		// Returns 0xFFFD on decoding error, consuming the minimal invalid subsequence
		// Precondition: pos < end.
		[[nodiscard]] constexpr std::uint32_t decode_utf8_char(
			unsigned char const*& pos,
			unsigned char const* end) noexcept
		{
			FSTLOG_ASSERT(pos != nullptr && pos < end);
			
			// --- Case 1: ASCII (1 byte: 0xxxxxxx) ---
			// Most common case in Western languages and logs.
			if (*pos < 0x80) {
				return *pos++;
			}

			// Load the first byte.
			const std::uint32_t b1 = *pos;
			// --- Case 2: 2 Bytes (110xxxxx 10xxxxxx) ---
			// Common: Latin-1 extensions (e.g., é, ü, ç, ß)
			// Lead byte check: 0b1110'0000 (0xE0) mask matches 0b1100'0000 (0xC0)
			if ((b1 & 0xE0) == 0xC0) {
				// Structural Check 1: Boundary
				if (end - pos < 2) {
					pos += 1; // Not enough data, consume lead byte only
					return utf_const::replacement_char;
				}

				const std::uint32_t b2 = *(pos + 1);

				// Structural Check 2: Continuation byte validity
				if ((b2 & utf_const::cont_prefix_mask) != utf_const::cont_prefix) {
					pos += 1; // Invalid continuation byte, consume lead byte only
					return utf_const::replacement_char;
				}

				// Decode
				std::uint32_t cp = ((b1 & 0x1F) << 6) | (b2 & utf_const::cont_data_mask);

				// Semantic Check: Overlong encoding (must be >= U+0080)
				if (cp < utf_const::min_2_byte_code_p) {
					pos += 2; // Structurally valid but semantically wrong, consume all
					return utf_const::replacement_char;
				}

				// Success
				pos += 2;
				return cp;
			}

			// --- Case 3: 3 Bytes (1110xxxx 10xxxxxx 10xxxxxx) ---
			// Less common: Euro symbol (€), some quotes, Greek, Cyrillic
			// Lead byte check: 0b1111'0000 (0xF0) mask matches 0b1110'0000 (0xE0)
			else if ((b1 & 0xF0) == 0xE0) {
				// Structural Check 1: Boundary
				if (end - pos < 3) {
					pos += 1; // Not enough data, consume lead byte only
					return utf_const::replacement_char;
				}

				const std::uint32_t b2 = *(pos + 1);
				const std::uint32_t b3 = *(pos + 2);

				// Structural Check 2: Continuation bytes (Minimal consumption rule)
				if ((b2 & utf_const::cont_prefix_mask) != utf_const::cont_prefix) {
					pos += 1; // b2 invalid, consume b1
					return utf_const::replacement_char;
				}
				if ((b3 & utf_const::cont_prefix_mask) != utf_const::cont_prefix) {
					pos += 2; // b3 invalid, consume b1 and valid b2
					return utf_const::replacement_char;
				}

				// Decode
				std::uint32_t cp = ((b1 & 0x0F) << 12)
					| ((b2 & utf_const::cont_data_mask) << 6)
					| (b3 & utf_const::cont_data_mask);

				// Semantic Checks:
				// 1. Overlong encoding (must be >= U+0800)
				// 2. Surrogates (U+D800 to U+DFFF are invalid)
				if (cp < utf_const::min_3_byte_code_p || (cp >= utf_const::surrogate_start && cp <= utf_const::surrogate_end)) {
					pos += 3; // Structurally valid but semantically wrong, consume all
					return utf_const::replacement_char;
				}

				// Success
				pos += 3;
				return cp;
			}

			// --- Case 4: 4 Bytes (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx) ---
			// Rare: Emojis, CJK, mathematical symbols
			// Lead byte check: 0b1111'1000 (0xF8) mask matches 0b1111'0000 (0xF0)
			else if ((b1 & 0xF8) == 0xF0) {
				// Structural Check 1: Boundary
				if (end - pos < 4) {
					pos += 1; // Not enough data, consume lead byte only
					return utf_const::replacement_char;
				}

				const std::uint32_t b2 = *(pos + 1);
				const std::uint32_t b3 = *(pos + 2);
				const std::uint32_t b4 = *(pos + 3);

				// Structural Check 2: Continuation bytes (Minimal consumption rule)
				if ((b2 & utf_const::cont_prefix_mask) != utf_const::cont_prefix) {
					pos += 1; // b2 invalid, consume b1
					return utf_const::replacement_char;
				}
				if ((b3 & utf_const::cont_prefix_mask) != utf_const::cont_prefix) {
					pos += 2; // b3 invalid, consume b1, b2
					return utf_const::replacement_char;
				}
				if ((b4 & utf_const::cont_prefix_mask) != utf_const::cont_prefix) {
					pos += 3; // b4 invalid, consume b1, b2, b3
					return utf_const::replacement_char;
				}

				// Decode
				std::uint32_t cp = ((b1 & 0x07) << 18)
					| ((b2 & utf_const::cont_data_mask) << 12)
					| ((b3 & utf_const::cont_data_mask) << 6)
					| (b4 & utf_const::cont_data_mask);

				// Semantic Checks:
				// 1. Overlong encoding (must be >= U+10000)
				// 2. Exceeding Unicode maximum (must be <= U+10FFFF)
				if (cp < utf_const::min_4_byte_code_p || cp > utf_const::max_code_p) {
					pos += 4; // Structurally valid but semantically wrong, consume all
					return utf_const::replacement_char;
				}

				// Success
				pos += 4;
				return cp;
			}

			// --- Case 5: Error (Invalid lead byte) ---
			// Handles bytes starting with 10xxxxxx (which are continuation bytes)
			// or 11111xxx (invalid sequences).
			else {
				pos += 1; // Consume the single invalid byte
				return utf_const::replacement_char;
			}
		}

		inline unsigned char* encode_safe_utf8_char(
			std::uint32_t code_point, 
			unsigned char* dest, 
			unsigned char const* dest_end) noexcept
		{
			FSTLOG_ASSERT(dest != nullptr);
			if (dest >= dest_end) return nullptr; // no space in buffer
			FSTLOG_ASSERT(valid_utf_code_point(code_point));

			if (!safe_utf_code_point(code_point)) {
				auto res = encode_escaped(code_point, dest, dest_end);
				if (res.ec != error_code::none) {
					return nullptr;
				}
				return res.ptr;
			}

			// 1 byte sequence (0xxxxxxx)
			if (code_point < utf_const::min_2_byte_code_p) {
				*dest = static_cast<unsigned char>(code_point);
				return dest + 1;
			}
			// 2 byte sequence (110xxxxx 10xxxxxx)
			if (code_point < utf_const::min_3_byte_code_p) {
				if(dest_end - dest < 2) return nullptr;
				*dest = static_cast<unsigned char>(
					(code_point >> 6) | std::uint32_t{ 0b1100'0000U });
				*(dest + 1) = static_cast<unsigned char>(
					(code_point & utf_const::cont_data_mask) | utf_const::cont_prefix);
				return dest + 2;
			}
			// 3 byte sequence (1110xxxx 10xxxxxx 10xxxxxx)
			if (code_point < utf_const::min_4_byte_code_p) {
				if (dest_end - dest < 3) return nullptr;
				*dest = static_cast<unsigned char>(
					(code_point >> 12) | std::uint32_t{ 0b1110'0000U });
				*(dest + 1) = static_cast<unsigned char>(
					((code_point >> 6) & utf_const::cont_data_mask) | utf_const::cont_prefix);
				*(dest + 2) = static_cast<unsigned char>(
					(code_point & utf_const::cont_data_mask) | utf_const::cont_prefix);
				return dest + 3;
			}
			// 4 byte sequence (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
			if (code_point <= utf_const::max_code_p) {
				if (dest_end - dest < 4) return nullptr;
				*dest = static_cast<unsigned char>(
					(code_point >> 18) | std::uint32_t{ 0b1111'0000U });
				*(dest + 1) = static_cast<unsigned char>(
					((code_point >> 12) & utf_const::cont_data_mask) | utf_const::cont_prefix);
				*(dest + 2) = static_cast<unsigned char>(
					((code_point >> 6) & utf_const::cont_data_mask) | utf_const::cont_prefix);
				*(dest + 3) = static_cast<unsigned char>(
					(code_point & utf_const::cont_data_mask) | utf_const::cont_prefix);
				return dest + 4;
			}
			return nullptr;
		}

		template <typename O>
		inline O* encode_utf16_char(
			std::uint32_t code_point, 
			O* dest, 
			O const* dest_end) noexcept 
		{
			FSTLOG_ASSERT(dest != nullptr && dest <= dest_end);
			static_assert(
				std::is_integral_v<O>
				&& (std::numeric_limits<O>::max)() >= 65'535, 
				"Invalid type!");
			FSTLOG_ASSERT(valid_utf_code_point(code_point) && "Invalid code point.");

			// fast one 16 bit value
			if (code_point <= 65'535) {
				if (dest >= dest_end) return nullptr; // no space in buffer
				*dest++ = static_cast<O>(code_point);
			}
			// encoding with surrogates
			else {
				if (dest_end - dest < 2) return nullptr; // no space in buffer
				code_point -= 0x10000;
				std::uint32_t surrogate_high = (code_point >> 10) + 0xD800;
				std::uint32_t surrogate_low = (code_point & 0x3FF) + 0xDC00;
				*dest = static_cast<O>(surrogate_high);
				*(dest + 1) = static_cast<O>(surrogate_low);
				dest += 2;
			}
			return dest;
		}

		template <typename I>
		[[nodiscard]] constexpr std::uint32_t decode_utf16_char(
			unsigned char const*& pos,
			unsigned char const* end) noexcept
		{
			FSTLOG_ASSERT(pos != nullptr && end > pos && sizeof(I) <= end - pos);
			rm_cvref_t<I> utf16_char{ 0 };
			memcpy(&utf16_char, pos, sizeof(I));
			if (!valid_utf16_char(utf16_char)) {
				pos += sizeof(I);
				return utf_const::replacement_char;
			}
			// non surrogate range
			if (utf16_char < 0xD800 || utf16_char > 0xDFFF) {
				pos += sizeof(I);
				return static_cast<std::uint32_t>(utf16_char);
			}

			// surrogate range
			
			// missing surrogate
			if (2 * sizeof(I) > end - pos) {
				pos += sizeof(I);
				return utf_const::replacement_char;
			}
			
			const std::uint32_t surrogate_high = static_cast<std::uint32_t>(utf16_char);
			memcpy(&utf16_char, pos + sizeof(I), sizeof(I));
			const std::uint32_t surrogate_low = static_cast<std::uint32_t>(utf16_char);
			// if surrogtes are valid
			if (surrogate_high <= 0xDBFF && surrogate_low >= 0xDC00 && surrogate_low <= 0xDFFF) {
				std::uint32_t code_p = ((surrogate_low & 0x3FF) | ((surrogate_high & 0x3FF) << 10)) + 0x10000;
				pos += 2 * sizeof(I);
				return code_p;
			}
			
			// invalid surrogates
			pos += sizeof(I);
			return utf_const::replacement_char;
		}

		template <typename I>
		inline buffer_operation_result<unsigned char> utf32_to_utf8(
			unsigned char const*& in,
			unsigned char const* input_end,
			unsigned char* dest,
			unsigned char const* dest_end,
			std::size_t& char_num) noexcept
		{
			FSTLOG_ASSERT(in != nullptr);
			std::size_t char_count{ 0 };
			while (in + sizeof(I) <= input_end && char_count != char_num) {
				rm_cvref_t<I> char_utf32{ 0 };
				memcpy(&char_utf32, in, sizeof(I));
				std::uint32_t code_point = static_cast<std::uint32_t>(char_utf32);
				if (!valid_utf32_char(char_utf32)) code_point = utf_const::replacement_char;
				if (!valid_utf_code_point(code_point)) code_point = utf_const::replacement_char;

				const auto next_dest = encode_safe_utf8_char(code_point, dest, dest_end);
				// encoding failed (no space)
				if (next_dest == nullptr) {
					char_num = char_count;
					return buffer_operation_result<unsigned char>{ dest, error_code::buff_full };
				}

				in += sizeof(I);
				dest = next_dest;
				char_count++;
			}
			char_num = char_count;
			return buffer_operation_result<unsigned char>{ dest, error_code::none };
		}

		template <typename I>
		inline buffer_operation_result<unsigned char> utf16_to_utf8(
			unsigned char const* &in,
			unsigned char const* input_end,
			unsigned char* dest,
			unsigned char const* dest_end,
			std::size_t& char_num) noexcept
		{
			FSTLOG_ASSERT(in != nullptr);
			static_assert(std::is_integral_v<I>, "Invalid type!");
			std::size_t char_count{ 0 };
			while (in < input_end && sizeof(I) <= input_end - in && char_count != char_num) {
				auto in_next = in;
				std::uint32_t code_p = decode_utf16_char<I>(in_next, input_end);
				const auto next_dest = encode_safe_utf8_char(code_p, dest, dest_end);
				// encoding failed, no space
				if (next_dest == nullptr) {
					char_num = char_count;
					return buffer_operation_result<unsigned char>{ 
						dest, error_code::buff_full };
				}
				in = in_next;
				dest = next_dest;
				char_count++;
			}
			char_num = char_count;
			return buffer_operation_result<unsigned char>{ dest, error_code::none };
		}

		template<typename I>
		inline detail::buffer_operation_result<unsigned char> utf8_to_utf8(
			const unsigned char*& input,
			const unsigned char* input_end,
			unsigned char* dest,
			const unsigned char* dest_end,
			std::size_t& char_num) noexcept
		{
			if (input == nullptr) {
				return detail::buffer_operation_result<unsigned char>{ dest, error_code::none };
			}
			static_assert(sizeof(I) == 1);
			
			auto ascii_end = input;
			while (ascii_end < input_end
				&& ((*ascii_end >= 0x20) & (*ascii_end < 0x7F)) != 0)
			{
				ascii_end++;
			}
			auto ascii_len = static_cast<std::size_t>(ascii_end - input);
			if (ascii_len > char_num) ascii_len = char_num;
			if (ascii_len > static_cast<std::size_t>(dest_end - dest)) ascii_len = static_cast<std::size_t>(dest_end - dest);
			memcpy(dest, input, ascii_len);
			std::size_t char_count = ascii_len;
			input += ascii_len;
			dest += ascii_len;
			
			while (input < input_end && char_count < char_num) {
				auto next_input = input;
				auto next_dest = dest;
				std::uint32_t code_point = decode_utf8_char(next_input, input_end);
				if (safe_utf_code_point(code_point)) {
					// 1 byte sequence (0xxxxxxx)
					if (code_point < utf_const::min_2_byte_code_p) {
						if (dest == dest_end) {
							char_num = char_count;
							return buffer_operation_result<unsigned char>{ dest, error_code::buff_full };
						}
						*dest = static_cast<unsigned char>(code_point);
						next_dest = dest + 1;
					}
					// 2 byte sequence (110xxxxx 10xxxxxx)
					else if (code_point < utf_const::min_3_byte_code_p) {
						if (dest_end - dest < 2) {
							char_num = char_count;
							return buffer_operation_result<unsigned char>{ dest, error_code::buff_full };
						}
						*dest = *input;
						*(dest + 1) = *(input + 1);
						next_dest = dest + 2;
					}
					// 3 byte sequence (1110xxxx 10xxxxxx 10xxxxxx)
					else if (code_point < utf_const::min_4_byte_code_p) {
						if (dest_end - dest < 3) {
							char_num = char_count;
							return buffer_operation_result<unsigned char>{ dest, error_code::buff_full };
						}
						*dest = *input;
						*(dest + 1) = *(input + 1);
						*(dest + 2) = *(input + 2);
						next_dest = dest + 3;
					}
					// 4 byte sequence (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
					else {
						if (dest_end - dest < 4) {
							char_num = char_count;
							return buffer_operation_result<unsigned char>{ dest, error_code::buff_full };
						}
						*dest = *input;
						*(dest + 1) = *(input + 1);
						*(dest + 2) = *(input + 2);
						*(dest + 3) = *(input + 3);
						next_dest = dest + 4;
					}
				}
				else {
					auto res = encode_escaped(code_point, dest, dest_end);
					if (res.ec != error_code::none) {
						char_num = char_count;
						return res;
					}
					next_dest = res.ptr;
				}
				input = next_input;
				dest = next_dest;
				char_count++;
			}
			char_num = char_count;
			return buffer_operation_result<unsigned char>{ dest, error_code::none };
		}
		
		template<typename O>
		inline buffer_operation_result<O> safe_utf8_to_utf16(
			unsigned char const* &input,
			unsigned char const* input_end,
			O* dest,
			O const* dest_end) noexcept
		{
			FSTLOG_ASSERT(input != nullptr);
			static_assert(
				std::is_integral_v<O>
				&& (std::numeric_limits<O>::max)() >= 65'535,
				"Invalid type!");
			while (input < input_end) {
				auto next_input = input;
				std::uint32_t code_point = decode_utf8_char(next_input, input_end);
				if (!safe_utf_code_point(code_point)) {
					return buffer_operation_result<O>{
						dest, error_code::input_bad };
				}
				
				O* next_dest = encode_utf16_char(code_point, dest, dest_end);
				// no space in buffer
				if (next_dest == nullptr) {
					return buffer_operation_result<O>{
						dest, error_code::buff_full };
				}
				input = next_input;
				dest = next_dest;
			}
			return buffer_operation_result<O>{ dest, error_code::none };
		}

        template<int utf_bits, typename I,
            std::enable_if_t< utf_bits == 16>* = nullptr>
        inline buffer_operation_result<unsigned char> utf8conv(
			unsigned char const*& in,
			unsigned char const* input_end,
			unsigned char* dest,
			unsigned char const* dest_end,
            std::size_t& char_num) noexcept 
        {
            return utf16_to_utf8<I>(in, input_end, dest, dest_end, char_num);
        }

        template<int utf_bits, typename I,
			std::enable_if_t< utf_bits == 32>* = nullptr>
        inline buffer_operation_result<unsigned char> utf8conv(
			unsigned char const*& in,
			unsigned char const* input_end,
			unsigned char* dest,
			unsigned char const* dest_end,
            std::size_t& char_num) noexcept
        {
            return utf32_to_utf8<I>(in, input_end, dest, dest_end, char_num);
        }
		
        template<int utf_bits, typename I,
            std::enable_if_t< utf_bits == 8>* = nullptr>
        inline buffer_operation_result<unsigned char> utf8conv(
			unsigned char const*& in,
			unsigned char const* input_end,
			unsigned char* dest,
			unsigned char const* dest_end,
            std::size_t& char_num) noexcept
        {
			return utf8_to_utf8<I>(in, input_end, dest, dest_end, char_num);
        }


		struct utf8_str_len {
			std::size_t byte_count;   // Number of bytes.
			std::size_t char_count;   // Number of unicode characters.
		};

		/*
		* @brief Calculates the length of a UTF-8 string (byte and char) trimmed to the desired char length.
		* If the string is shorter than the trim length, the full lengths are returned.
		*
		* @param begin/end Pointers defining the input UTF-8 buffer [begin, end).
		* @param trim_length The desired trimmed length (in utf characters). 
		* @return {byte_count, char_count} where:
		*         - byte_count: byte length of the trimmed string
		*         - char_count: character length of the trimmed string
		* @tparam T Must satisfy sizeof(T) == 1 (e.g., char, unsigned char, char8_t).
		*/
		template<typename T>
		inline utf8_str_len utf8_str_trim(
			T const* begin,
			T const* end,
			std::size_t trim_length = (std::numeric_limits<std::size_t>::max)()) noexcept 
		{
			static_assert(sizeof(T) == 1, "Invalid type!");
			FSTLOG_ASSERT(begin != nullptr);
			const auto str_begin = safe_reinterpret_cast<const unsigned char*>(begin);
			const auto str_end = safe_reinterpret_cast<const unsigned char*>(end);
			auto pos = str_begin;
			
			// ASCII fast path
			while (pos < str_end && *pos < 0x80) pos++;
			std::size_t char_num{ static_cast<std::size_t>(pos - str_begin) };
			if (char_num >= trim_length) {
				return { trim_length, trim_length };
			}

			while (pos < str_end && char_num != trim_length) {
				[[maybe_unused]] auto code_point = 
					detail::decode_utf8_char(pos, str_end);
				char_num++;
			}

			return { static_cast<std::size_t>(pos - str_begin), char_num };
		}
    }
}
