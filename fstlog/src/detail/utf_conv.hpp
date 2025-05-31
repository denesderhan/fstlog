//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstring>
#include <limits>
#pragma intrinsic(memcpy)

#include <detail/buffer_operation_result.hpp>
#include <fstlog/detail/rm_cvref_t.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <detail/utf8_helper.hpp>

namespace fstlog {
	namespace detail {
		
		template <typename T>
		inline bool valid_code_point(T code_point) noexcept {
			static_assert(std::is_integral_v<T>, "Invalid type!");
			// valid UTF-32 code points: 0 - 56'319, 57'344 - 1'114'111
			if constexpr ((std::numeric_limits<T>::min)() < 0){
				if (code_point < 0) return false;
			}
			if (code_point <= 56'319) return true;
			if constexpr ((std::numeric_limits<T>::max)() > 1'114'111) {
				if (code_point > 1'114'111) return false;
			}
			if (code_point >= 57'344) return true;
			return false;
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

		template <typename O>
		inline O* encode_utf8(std::uint32_t code_point, O* dest, O* dest_end) noexcept {
			FSTLOG_ASSERT(dest != nullptr);
			if (dest >= dest_end) return nullptr; // no space in buffer
			static_assert(
				std::is_integral_v<O>
				&& (std::numeric_limits<O>::max)() >= 255,
				"Invalid type!");
			FSTLOG_ASSERT(valid_code_point(code_point) && "Invalid code point.");

			unsigned int first_byte = 0;
			int continuation_bytes = 0;
			// 1 byte
			if (code_point < 0x80) {								
				*dest = static_cast<O>(code_point);
				return dest + 1;
			}
			// 2 byte
			else if (code_point < 0x800) {
				first_byte = 0b1100'0000;
				continuation_bytes = 1;	
			}
			// 3 byte
			else if (code_point < 0x10000) {
				first_byte = 0b1110'0000;
				continuation_bytes = 2;	
			}
			// 4 byte
			else {
				first_byte = 0b1111'0000;
				continuation_bytes = 3;							
			}
			if (dest + continuation_bytes >= dest_end) return nullptr;	// no space in buffer
			
			int bit_shift_num = continuation_bytes * 6;
			// write first byte
			*dest++ = static_cast<O>((code_point >> bit_shift_num) | first_byte);
			while (continuation_bytes-- > 0) {
				bit_shift_num -= 6;
				*dest++ = static_cast<O>(((code_point >> bit_shift_num) & 0b0011'1111) | 0b1000'0000);
			}
			return dest;
		}

		template <typename I>
		inline unsigned char const* decode_utf8(
			std::uint32_t& code_point,
			unsigned char const* pos,
			unsigned char const* end) noexcept
		{
			FSTLOG_ASSERT(pos != nullptr && pos + sizeof(I) <= end);
			static_assert(std::is_integral_v<I>, "Invalid type!");
			rm_cvref_t<I> utf8_char{ 0 };
			memcpy(&utf8_char, pos, sizeof(I));
			if (!valid_utf8_char(utf8_char)) return nullptr;
			pos += sizeof(I);
			const std::uint32_t first_byte = static_cast<std::uint32_t>(utf8_char);
			
			// 1 byte 0xxx xxxx									 (0 - 127 ASCII)
			// 2 byte 110x xxxx  10xx xxxx						 (128 - 2047)
			// 3 byte 1110 xxxx  10xx xxxx  10xx xxxx			 (2048 - 65535)
			// 4 byte 1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx (65536 - 1114111)

			// 1 byte
			if ((first_byte & 0b1000'0000) == 0) {
				code_point = first_byte;
				return pos;
			}

			// multi byte
			int continuation_bytes = 0;
			std::uint32_t code_p = 0;
			// 2 byte
			if ((first_byte & 0b1110'0000) == 0b1100'0000) {
				continuation_bytes = 1;
				// init code_point with the first bytes data
				code_p = first_byte & 0b0001'1111;
			}
			// 3 byte
			else if ((first_byte & 0b1111'0000) == 0b1110'0000) {
				continuation_bytes = 2;
				// init code_point with the first bytes data
				code_p = first_byte & 0b0000'1111;
			}
			// 4 byte
			else if ((first_byte & 0b1111'1000) == 0b1111'0000) {
				continuation_bytes = 3;
				// init code_point with the first bytes data
				code_p = first_byte & 0b0000'0111;
			}
			else {
				// invalid data in first byte
				return nullptr;	
			}
			// missing data
			if (pos + continuation_bytes * sizeof(I) > end) return nullptr;

			// add continuation byte's 6 bit data
			while (continuation_bytes-- > 0) {
				memcpy(&utf8_char, pos, sizeof(I));
				pos += sizeof(I);
				if (!valid_utf8_char(utf8_char)) return nullptr;
				std::uint32_t byte = static_cast<std::uint32_t>(utf8_char);
				// integrity check
				if ((byte & 0b1100'0000) != 0b1000'0000) return nullptr;
				code_p <<= 6;
				code_p += byte & 0b0011'1111;
			}

			if (valid_code_point(code_p)) {
				code_point = code_p;
				return pos;
			}
			else {
				return nullptr;
			}
		}

		template <typename O>
		inline O* encode_utf16(std::uint32_t code_point, O* dest, O* dest_end) noexcept {
			FSTLOG_ASSERT(dest != nullptr);
			static_assert(
				std::is_integral_v<O>
				&& (std::numeric_limits<O>::max)() >= 65'535, 
				"Invalid type!");
			FSTLOG_ASSERT(valid_code_point(code_point) && "Invalid code point.");

			// fast one 16 bit value
			if (code_point <= 65'535) {
				if (dest >= dest_end) return nullptr; // no space in buffer
				*dest++ = static_cast<O>(code_point);
			}
			// encoding with surrogates
			else {
				if (dest + 2 > dest_end) return nullptr; // no space in buffer
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
		inline unsigned char const* decode_utf16(
			std::uint32_t& code_point,
			unsigned char const* pos,
			unsigned char const* end) noexcept
		{
			FSTLOG_ASSERT(pos != nullptr && pos + sizeof(I) <= end);
			rm_cvref_t<I> utf16_char{ 0 };
			memcpy(&utf16_char, pos, sizeof(I));
			if (!valid_utf16_char(utf16_char)) return nullptr;
			std::uint32_t code_p = 0;
			if (utf16_char < 0xD800 || utf16_char > 0xDFFF) {
				code_p = static_cast<std::uint32_t>(utf16_char);
				pos += sizeof(I);
			}
			else {
				// missing surrogate
				if (pos + 2 * sizeof(I) > end) return nullptr;
				const std::uint32_t surrogate_high = static_cast<std::uint32_t>(utf16_char);
				memcpy(&utf16_char, pos + sizeof(I), sizeof(I));
				const std::uint32_t surrogate_low = static_cast<std::uint32_t>(utf16_char);
				// if surrogtes are valid
				if (surrogate_high <= 0xDBFF && surrogate_low >= 0xDC00 && surrogate_low <= 0xDFFF) {
					code_p = ((surrogate_low & 0x3FF) | ((surrogate_high & 0x3FF) << 10)) + 0x10000;
					pos += 2 * sizeof(I);
				}
				else {
					return nullptr;
				}
			}
			code_point = code_p;
			return pos;
		}

		template <typename I, typename O>
		inline buffer_operation_result<O> utf32_to_utf8(
			unsigned char const*& in,
			unsigned char const* input_end,
			O* dest,
			O* dest_end,
			std::size_t& char_num) noexcept
		{
			FSTLOG_ASSERT(in != nullptr);
			std::size_t char_count{ 0 };
			while (in + sizeof(I) <= input_end && char_count != char_num) {
				rm_cvref_t<I> code_point{ 0 };
				memcpy(&code_point, in, sizeof(I));
				// encoding failed invalid code_point
				if (!valid_code_point(code_point)) {
					char_num = char_count;
					return buffer_operation_result<O>{
						dest, error_code::input_bad };
				}

				O* next_dest = encode_utf8(static_cast<std::uint32_t>(code_point), dest, dest_end);
				// encoding failed (no space)
				if (next_dest == nullptr) {
					char_num = char_count;
					return buffer_operation_result<O>{ dest, error_code::buff_full };
				}

				in += sizeof(I);
				dest = next_dest;
				char_count++;
			}
			char_num = char_count;
			return buffer_operation_result<O>{ dest, error_code::none };
		}

		template <typename I, typename O>
		inline buffer_operation_result<O> utf16_to_utf8(
			unsigned char const* &in,
			unsigned char const* input_end,
			O* dest,
			O* dest_end,
			std::size_t& char_num) noexcept
		{
			FSTLOG_ASSERT(in != nullptr);
			static_assert(std::is_integral_v<I>, "Invalid type!");
			std::size_t char_count{ 0 };
			while (in + sizeof(I) <= input_end && char_count != char_num) {
				std::uint32_t code_p{ 0 };
				unsigned char const* next_in = decode_utf16<I>(code_p, in, input_end);
				// decoding failed, bad or missing data
				if (next_in == nullptr) {
					char_num = char_count;
					return buffer_operation_result<O>{
						dest, error_code::input_bad };
				}
				O* next_dest = encode_utf8(code_p, dest, dest_end);
				// encoding failed, no space
				if (next_dest == nullptr) {
					char_num = char_count;
					return buffer_operation_result<O>{ 
						dest, error_code::buff_full };
				}
				in = next_in;
				dest = next_dest;
				char_count++;
			}
			char_num = char_count;
			return buffer_operation_result<O>{ dest, error_code::none };
		}

		template<typename I, typename O>
		inline buffer_operation_result<O> utf8_to_utf32(
			unsigned char const*& input,
			unsigned char const* input_end,
			O* dest,
			O* dest_end,
			std::size_t& char_num) noexcept
		{
			FSTLOG_ASSERT(input != nullptr);
			static_assert(
				std::is_integral_v<O>
				&& (std::numeric_limits<O>::max)() >= 1'114'111,
				"Invalid type!");
			std::size_t char_count{ 0 };
			while (input + sizeof(I) <= input_end && char_count != char_num) {
				if (dest >= dest_end) {
					char_num = char_count;
					return buffer_operation_result<O>{
						dest, error_code::buff_full };
				}
				std::uint32_t code_point{0};
				unsigned char const* next_input = decode_utf8<I>(code_point, input, input_end);
				// bad or missing input data
				if (next_input == nullptr) {
					char_num = char_count;
					return buffer_operation_result<O>{
						dest, error_code::input_bad };
				}

				*dest++ = static_cast<O>(code_point);
				input = next_input;
				char_count++;
			}
			char_num = char_count;
			return buffer_operation_result<O>{ dest, error_code::none };
		}

		template<typename I, typename O>
		inline buffer_operation_result<O> utf8_to_utf16(
			unsigned char const*& input,
			unsigned char const* input_end,
			O* dest,
			O* dest_end,
			std::size_t& char_num) noexcept
		{
			FSTLOG_ASSERT(input != nullptr);
			static_assert(
				std::is_integral_v<O>
				&& (std::numeric_limits<O>::max)() >= 65'535,
				"Invalid type!");
			std::size_t char_count{ 0 };
			while (input + sizeof(I) <= input_end && char_count != char_num) {
				std::uint32_t code_point{ 0 };
				unsigned char const* next_input = decode_utf8<I>(code_point, input, input_end);
				// bad or missing input data
				if (next_input == nullptr) {
					char_num = char_count;
					return buffer_operation_result<O>{
						dest, error_code::input_bad };
				}
				
				O* next_dest = encode_utf16(code_point, dest, dest_end);
				// no space in buffer
				if (next_dest == nullptr) {
					char_num = char_count;
					return buffer_operation_result<O>{
						dest, error_code::buff_full };
				}

				input = next_input;
				dest = next_dest;
				char_count++;
			}
			char_num = char_count;
			return buffer_operation_result<O>{ dest, error_code::none };
		}

        template<int utf_bits, typename I, typename O,
            std::enable_if_t< utf_bits == 16>* = nullptr>
        inline buffer_operation_result<O> utf8conv(
			unsigned char const*& in,
			unsigned char const* input_end,
            O* dest,
            O* dest_end,
            std::size_t& char_num) noexcept 
        {
            return utf16_to_utf8<I, O>(in, input_end, dest, dest_end, char_num);
        }

        template<int utf_bits, typename I, typename O,
            std::enable_if_t< utf_bits == 32>* = nullptr>
        inline buffer_operation_result<O> utf8conv(
			unsigned char const*& in,
			unsigned char const* input_end,
            O* dest,
            O* dest_end,
            std::size_t& char_num) noexcept
        {
            return utf32_to_utf8<I, O>(in, input_end, dest, dest_end, char_num);
        }
		
        template<int utf_bits, typename I, typename O,
            std::enable_if_t< utf_bits == 8>* = nullptr>
        inline buffer_operation_result<O> utf8conv(
			unsigned char const*& in,
			unsigned char const* input_end,
            O* dest,
            O* dest_end,
            std::size_t& char_num) noexcept
        {
            return utf8cpy<I, O>(in, input_end, dest, dest_end, char_num);
        }
    }
}
