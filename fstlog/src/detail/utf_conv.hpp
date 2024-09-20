//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <limits>
#include <cstring>
#pragma intrinsic(memcpy)

#include <detail/buffer_operation_result.hpp>
#include <fstlog/detail/rm_cvref_t.hpp>
#include <detail/utf8_helper.hpp>

namespace fstlog {
    namespace detail {
        namespace {
            inline constexpr std::uint32_t utf_surrogate_high_min = std::uint32_t{ 0xD800 }; //55 296
            inline constexpr std::uint32_t utf_surrogate_high_max = std::uint32_t{ 0xDBFF }; //56 319
            inline constexpr std::uint32_t utf_surrogate_low_min = std::uint32_t{ 0xDC00 }; //56 320
            inline constexpr std::uint32_t utf_surrogate_low_max = std::uint32_t{ 0xDFFF }; //57 343
            inline constexpr std::uint32_t max_valid_utf32 = std::uint32_t{ 0x0010FFFF }; //1 114 111
            inline constexpr std::uint32_t utf_byte_mask = std::uint32_t{ 0x3F }; //0b0011 1111
            inline constexpr std::uint32_t utf_byte_mark = std::uint32_t{ 0x80 }; //0b1000 0000
        }
        
        template<typename I, typename O>
        inline buffer_operation_result<O> utf32_to_utf8(
			unsigned char const*& in,
			unsigned char const* input_end,
            O* dest,
            O* dest_end,
            std::size_t& char_num) noexcept
		{
			if (in == nullptr) {
				return buffer_operation_result<O>{ dest, error_code::none };
			}
			static_assert(
				std::is_integral_v<I>
				&& (std::numeric_limits<I>::max)() >= max_valid_utf32,
                "Invalid type!");
            static_assert(
				std::is_integral_v<O> 
				&& (std::numeric_limits<O>::max)() >= (std::numeric_limits<std::uint8_t>::max)(),
                "Invalid type!");
            std::size_t num{ 0 };
            while (in + sizeof(I) <= input_end && num != char_num) {
				rm_cvref_t<I> input_num{ 0 };
                memcpy(
                    &input_num,
                    in,
                    sizeof(I));
                std::uint32_t ch = input_num;
                //ascii (1 byte utf8) (0 - 127)
                if (ch < std::uint32_t{ 0x80 }) {
                    if (dest >= dest_end) {
                        char_num = num;
                        return buffer_operation_result<O>{
                            dest, error_code::no_space_in_buffer};
                    }
                    *dest++ = static_cast<O>(ch);
                    num++;
					in += sizeof(I);
                    continue;
                }
                //2 byte    (128 - 2047)
                if (ch < std::uint32_t{ 0x800 }) {
                    if (dest + 2 > dest_end) {
                        char_num = num;
                        return buffer_operation_result<O>{
                            dest, 
							error_code::no_space_in_buffer};
                    }
                    *dest = static_cast<O>(
                        ((ch >> 6) & std::uint32_t{ 0x1F }) 
                        | std::uint32_t{ 0xC0 });
                    *(dest + 1) = static_cast<O>(
                        (ch & utf_byte_mask) | utf_byte_mark);
                    dest += 2;
                    num++;
					in += sizeof(I);
                    continue;
                }
                //3 byte (2048 - 65535) 
                if (ch < std::uint32_t{ 0x10000 }) {
                    if (dest + 3 > dest_end) {
                        char_num = num;
                        return buffer_operation_result<O>{
                            dest, error_code::no_space_in_buffer };
                    }
                    if (ch < utf_surrogate_high_min || ch > utf_surrogate_low_max) {
                        *dest = static_cast<O>(
                            ((ch >> 12) & std::uint32_t{ 0x0F }) | std::uint32_t{ 0xE0 });
                        *(dest + 1) = static_cast<O>(
                            ((ch >> 6) & utf_byte_mask) | utf_byte_mark);
                        *(dest + 2) = static_cast<O>(
                            (ch & utf_byte_mask) | utf_byte_mark);
                        dest += 3;
                        num++;
						in += sizeof(I);
                        continue;
                    }
                }
                //4 byte (65536 - 1114111 ) (do not "fix" else if to if)
                else if (ch <= max_valid_utf32) {
                    if (dest + 4 > dest_end) {
                        char_num = num;
                        return buffer_operation_result<O>{
                            dest, error_code::no_space_in_buffer };
                    }
                    *dest = static_cast<O>(
                        ((ch >> 18) & std::uint32_t{ 0x07 }) | std::uint32_t{ 0xF0 });
                    *(dest + 1) = static_cast<O>(
                        ((ch >> 12) & utf_byte_mask) | utf_byte_mark);
                    *(dest + 2) = static_cast<O>(
                        ((ch >> 6) & utf_byte_mask) | utf_byte_mark);
                    *(dest + 3) = static_cast<O>(
                        (ch & utf_byte_mask) | utf_byte_mark);
                    dest += 4;
                    num++;
					in += sizeof(I);
                    continue;
                }
				//abort
				{
					char_num = num;
					return buffer_operation_result<O>{
						dest, error_code::input_contract_violation };
				}
            }
            char_num = num;
            return buffer_operation_result<O>{ dest, error_code::none };
        }

        template<typename I, typename O>
        inline buffer_operation_result<O> utf16_to_utf8(
			unsigned char const*& in,
			unsigned char const* input_end,
            O* dest,
            O* dest_end,
            std::size_t& char_num) noexcept
		{
			if (in == nullptr) {
				return buffer_operation_result<O>{ dest, error_code::none };
			}
			static_assert(
				std::is_integral_v<I>
				&& (std::numeric_limits<I>::max)() >= (std::numeric_limits<std::uint16_t>::max)(),
                "Invalid type!");
            static_assert(
				std::is_integral_v<O>
				&& (std::numeric_limits<O>::max)() >= (std::numeric_limits<std::uint8_t>::max)(),
                "Invalid type!");
            std::size_t num{ 0 };
            while (in + sizeof(I) <= input_end && num != char_num ) {
				rm_cvref_t<I> input_num{0};
                memcpy(
                    &input_num, 
                    in, 
                    sizeof(I));
                std::uint32_t current_char = input_num;
                //ascii (1 byte utf8) (0 - 127)
                if (current_char < 0x80) {
                    if (dest >= dest_end) {
                        char_num = num;
                        return buffer_operation_result<O>{
                            dest, error_code::no_space_in_buffer };
                    }
                    *dest++ = static_cast<O>(current_char);
                    in += sizeof(I);
                    num++;
                    continue;
                }
                //2 byte (128 - 2047)
                if (current_char < 0x800) {
                    if (dest + 2 > dest_end) {
                        char_num = num;
                        return buffer_operation_result<O>{
                            dest, error_code::no_space_in_buffer };
                    }
                    *dest = static_cast<O>(
                        ((current_char >> 6) & std::uint32_t{ 0x1F }) | std::uint32_t{ 0xC0 });
                    *(dest + 1) = static_cast<O>(
                        (current_char & utf_byte_mask) | utf_byte_mark);
                    dest += 2;
					in += sizeof(I);
                    num++; 
                    continue;
                }
                //3 byte (2048 - 65535 non surrogate) 
                if (current_char < utf_surrogate_high_min
                    || current_char > utf_surrogate_low_max) 
                {
                    if (dest + 3 > dest_end) {
                        char_num = num; 
                        return buffer_operation_result<O>{
                            dest, error_code::no_space_in_buffer };
                    }
                    *dest = static_cast<O>(
                        ((current_char >> 12) & std::uint32_t{ 0x0F }) | 
                        std::uint32_t{ 0xE0 });
                    *(dest + 1) = static_cast<O>(
                        ((current_char >> 6) & utf_byte_mask) | utf_byte_mark);
                    *(dest + 2) = static_cast<O>(
                        (current_char & utf_byte_mask) | utf_byte_mark);
                    dest += 3;
					in += sizeof(I);
                    num++;
                    continue;
                }
                //4 byte (65536 - 1114111 surrogate p.)
                if (current_char <= utf_surrogate_high_max
                    && in + 2 * sizeof(I) <= input_end) 
                {
                    const std::uint32_t surr_high = current_char;
					rm_cvref_t<I> next_char{ 0 };
                    memcpy(
                        &next_char, 
                        in + sizeof(I),
                        sizeof(I)
                    );
                    const std::uint32_t surr_low = next_char;
                    if (surr_low >= utf_surrogate_low_min && 
                        surr_low <= utf_surrogate_low_max) 
                    {
                        if (dest + 4 > dest_end) {
                            char_num = num;
                            return buffer_operation_result<O>{ 
                                dest, error_code::no_space_in_buffer };
                        }
                        const std::uint32_t ch = ((surr_low & std::uint32_t{ 0x3FF }) |
                            ((surr_high & std::uint32_t{ 0x3FF }) << 10)) +
                            std::uint32_t{ 0x10000 };

                        *dest = static_cast<O>(
                            ((ch >> 18) & std::uint32_t{ 0x07 }) | std::uint32_t{ 0xF0 });
                        *(dest + 1) = static_cast<O>(
                            ((ch >> 12) & utf_byte_mask) | utf_byte_mark);
                        *(dest + 2) = static_cast<O>(
                            ((ch >> 6) & utf_byte_mask) | utf_byte_mark);
                        *(dest + 3) = static_cast<O>(
                                (ch & utf_byte_mask) | utf_byte_mark);
                        dest += 4;
                        in += 2 * sizeof(I);
                        num++;
                        continue;
                    }
                }
                //abort
                {
					char_num = num;
					return buffer_operation_result<O>{
						dest, error_code::input_contract_violation };
                }
            }
            char_num = num;
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
			static_assert(sizeof(I) == 1, "Only 1 byte chars!");
			if (input == nullptr) {
				return buffer_operation_result<O>{ dest, error_code::none };
			}
			static_assert(
				std::is_integral_v<O>
                && (std::numeric_limits<O>::max)() >= max_valid_utf32,
                "Invalid type!");
           std::size_t num{ 0 };
            while (input < input_end && num != char_num) {
                if (dest >= dest_end) {
                    char_num = num;
                    return buffer_operation_result<O>{ 
                        dest, error_code::no_space_in_buffer };
                }
                //ascii (1 byte utf8) 0xxx xxxx
				if (*input < 0x80) {
                    *dest++ = static_cast<O>(*input++);
                    num++;
                    continue;
                }
                //invalid utf8 10xx xxxx
                if (*input < 0xC0) {
                    //fall through to abort
                }
                //2 byte 110x xxxx  10xx xxxx (128 - 2047)
                else if (*input < 0xE0
                    && *input > 0xC1
                    && input + 2 <= input_end
                    && (static_cast<std::uint32_t>(*(input + 1)) >> 6) == 2)
                {
                    std::uint32_t ch1 = *input;
                    std::uint32_t ch0 = *(input + 1);
                    std::uint32_t ch = 
                        ((ch1 & std::uint32_t{ 0x1F }) << 6)
                        | (ch0 & utf_byte_mask);
                    *dest++ = static_cast<O>(ch);
                    input += 2;
                    num++;
                    continue;
                }
                //3 byte 1110 xxxx  10xx xxxx  10xx xxxx (2048 - 65535)
                else if (*input < 0xF0
                    && input + 3 <= input_end
                    && (static_cast<std::uint32_t>(*(input + 1)) >> 6) == 2
                    && (static_cast<std::uint32_t>(*(input + 2)) >> 6) == 2)
                {
                    std::uint32_t ch2 = *input;
                    std::uint32_t ch1 = *(input + 1);
                    std::uint32_t ch0 = *(input + 2);
                    std::uint32_t ch = 
						((ch2 & std::uint32_t{ 0x0F }) << 12)
                        | ((ch1 & utf_byte_mask) << 6)
                        | (ch0 & utf_byte_mask);
                    if ((ch > 0x7FF && ch < utf_surrogate_high_min) 
                        || ch > utf_surrogate_low_max) 
                    {
                        *dest++ = static_cast<O>(ch);
                        input += 3;
                        num++;
                        continue;
                    }
                }
                //4 byte 1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx (65536 - 1114111)
                else if (*input < 0xF8
                    && input + 4 <= input_end
                    && (static_cast<std::uint32_t>(*(input + 1)) >> 6) == 2
                    && (static_cast<std::uint32_t>(*(input + 2)) >> 6) == 2
                    && (static_cast<std::uint32_t>(*(input + 3)) >> 6) == 2)
				{

                    std::uint32_t ch3 = *input;
                    std::uint32_t ch2 = *(input + 1);
                    std::uint32_t ch1 = *(input + 2);
                    std::uint32_t ch0 = *(input + 3);
                    std::uint32_t ch = 
						((ch3 & std::uint32_t{ 0x07 }) << 18)
                        | ((ch2 & utf_byte_mask) << 12)
                        | ((ch1 & utf_byte_mask) << 6)
                        | (ch0 & utf_byte_mask);
                    if (ch > 0xFFFF && ch <= max_valid_utf32) {
                        *dest++ = static_cast<O>(ch);
                        input += 4;
                        num++;
                        continue;
                    }
                }
				//abort
				{
					char_num = num;
					return buffer_operation_result<O>{
						dest, error_code::input_contract_violation };
				}
            }
            char_num = num;
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
			static_assert(sizeof(I) == 1, "Only 1 byte chars!");
			if (input == nullptr)
				return buffer_operation_result<O>{ dest, error_code::none };
			static_assert(
				std::is_integral_v<O>
                && (std::numeric_limits<O>::max)() >= (std::numeric_limits<std::uint16_t>::max)(),
                "Invalid type!"); 
            std::size_t num{ 0 };
            while (input < input_end && num != char_num ) {
                if (dest >= dest_end) {
                    char_num = num;
                    return buffer_operation_result<O>{ 
                        dest, error_code::no_space_in_buffer };
                }
                //ascii (1 byte utf8) 0xxx xxxx (0 - 127)
                if (*input < 0x80) {
                    *dest++ = static_cast<O>(*input++);
                    num++;
                    continue;
                }
                //invalid utf8
                if (*input < 0xC0) {
                    //fall through to abort
                }
                //2 byte 110x xxxx  10xx xxxx (128 - 2047)
                else if (*input < 0xE0
                    && *input > 0xC1
                    && input + 2 <= input_end
                    && (static_cast<std::uint32_t>(*(input + 1)) >> 6) == 2)
                {
                    std::uint32_t ch1 = *input;
                    std::uint32_t ch0 = *(input + 1);
                    std::uint32_t ch = 
                        ((ch1 & std::uint32_t{ 0x1F }) << 6)
                        | (ch0 & utf_byte_mask);
                    *dest++ = static_cast<O>(ch);
                    input += 2;
                    num++;
                    continue;
                }
                //3 byte 1110 xxxx  10xx xxxx  10xx xxxx (2048 - 65535)
                else if (*input < 0xF0
                    && input + 3 <= input_end
                    && (*(input + 1) >> 6) == 2
                    && (*(input + 2) >> 6) == 2)
                {
                    std::uint32_t ch2 = *input;
                    std::uint32_t ch1 = *(input + 1);
                    std::uint32_t ch0 = *(input + 2);
                    std::uint32_t ch = 
						((ch2 & std::uint32_t{ 0x0F }) << 12)
                        | ((ch1 & utf_byte_mask) << 6)
                        | (ch0 & utf_byte_mask);
                    if ((ch > 0x7FF && ch < utf_surrogate_high_min) 
                        || ch > utf_surrogate_low_max) 
                    {
                        *dest++ = static_cast<O>(ch);
                        input += 3;
                        num++;
                        continue;
                    }
                }
                //4 byte 1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx (65536 - 1114111)
                else if (*input < 0xF8
                    && input + 4 <= input_end
                    && (static_cast<std::uint32_t>(*(input + 1)) >> 6) == 2
                    && (static_cast<std::uint32_t>(*(input + 2)) >> 6) == 2
                    && (static_cast<std::uint32_t>(*(input + 3)) >> 6) == 2)
                {
                    std::uint32_t ch3 = *input;
                    std::uint32_t ch2 = *(input + 1);
                    std::uint32_t ch1 = *(input + 2);
                    std::uint32_t ch0 = *(input + 3);
                    std::uint32_t ch = 
                        ((ch3 & std::uint32_t{ 0x07 }) << 18)
                        | ((ch2 & utf_byte_mask) << 12)
                        | ((ch1 & utf_byte_mask) << 6)
                        | (ch0 & utf_byte_mask);
                    if (ch > 0xFFFF && ch <= max_valid_utf32) {
                        if (dest + 2 > dest_end) {
                            char_num = num;
                            return buffer_operation_result<O>{ 
                                dest, error_code::no_space_in_buffer };
                        }
                        //subtract prev. covered utf range 
                        ch -= std::uint32_t{ 0x0010000 };
                        std::uint32_t surr_high = 
                            ((ch >> 10) & std::uint32_t{ 0x3FF }) | std::uint32_t{ 0xD800 };
                        std::uint32_t surr_low = 
                            (ch & std::uint32_t{ 0x3FF }) | std::uint32_t{ 0xDC00 };
                        *dest = static_cast<O>(surr_high);
                        *(dest + 1) = static_cast<O>(surr_low);
                        dest += 2;
                        input += 4;
                        num++;
                        continue;
                    }
                }
				//abort
				{
					char_num = num;
					return buffer_operation_result<O>{
						dest, error_code::input_contract_violation };
				}
            }
            char_num = num;
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
