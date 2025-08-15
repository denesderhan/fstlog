//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstring>
#include <limits>
#include <cstdint>
#include <type_traits>
#pragma intrinsic(memcpy)

#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
    namespace detail {
        template <typename T>
        constexpr inline std::make_unsigned_t<T> abs_unsigned(T data) noexcept {
            static_assert(std::is_integral_v<T>, "Unsupported type!");
            if constexpr (std::is_unsigned_v<T>) {
                return data;
            }
            else {
                static_assert((-1 & 3) == 3, "Not twos complement!");
                std::make_unsigned_t<T> data_bits{ 0 };
                memcpy(&data_bits, &data, sizeof(T));
                // calculate  two's complement
                if (data < 0) return ~data_bits + 1;
                else return data_bits;
            }
        }

        inline void num_to_upper_case(char* begin, char* end) noexcept {
            while (begin < end) {
                if (*begin >= 'a') *begin -= ('a' - 'A');
                begin++;
            }
        }

        inline void write_sign(
            unsigned char sign_fmt,
            bool num_negative,
            unsigned char*& pos) noexcept
        {
            FSTLOG_ASSERT(sign_fmt == '-' || sign_fmt == '+' || sign_fmt == ' ');
            if (sign_fmt == '-') {
                if (!num_negative) return;
            } 
            else {
                if (num_negative) sign_fmt = '-';
            }
            *pos++ = sign_fmt;
        }
        
        inline void int_write_prefix(int base, unsigned char base_char, unsigned char* &pos) noexcept {
            if (base == 10) return;
            *pos++ = '0';
            if (base_char != 'o') *pos++ = base_char;
        }

        constexpr inline unsigned char sanitize_int_type_char(unsigned char type_char) noexcept {
            if (type_char >= 64 && type_char < 128) {
                constexpr std::uint64_t lut =
                    (1ULL << ('x' - 64)) | (1ULL << ('X' - 64))
                    | (1ULL << ('b' - 64)) | (1ULL << ('B' - 64))
                    | (1ULL << ('d' - 64)) | (1ULL << ('o' - 64));
                if ((lut >> (type_char - 64)) & 1) return type_char;
            }
            return 0;
        }

        constexpr inline unsigned char sanitize_float_type_char(unsigned char type_char) noexcept {
            if (type_char >= 64 && type_char < 128) {
                constexpr std::uint64_t lut =
                    (1ULL << ('a' - 64)) | (1ULL << ('A' - 64))
                    | (1ULL << ('e' - 64)) | (1ULL << ('E' - 64))
                    | (1ULL << ('f' - 64)) | (1ULL << ('F' - 64))
                    | (1ULL << ('g' - 64)) | (1ULL << ('G' - 64));
                if ((lut >> (type_char - 64)) & 1) return type_char;
            }
            return 0;
        }

        constexpr inline int get_int_base(unsigned char sanitized_type_char) noexcept {
            constexpr std::uint64_t lut =
                0b00001000'00001010'00001010'00010000'00001010'00001010'00000010'00001010;
                //8(o),    10(?),   10(?),    16(X,x), 10(?),   10(d),    2(B,b),  10(/0)
            return (lut >> (((sanitized_type_char >> 1) & 0b111) << 3)) & 0b1111'1111;
        }

        constexpr inline int charconv_float_format(unsigned char sanitized_type_char) noexcept {
            constexpr std::uint32_t lut =
                0b0011'0010'0001'0011'0011'0011'0100'0011;
                //gen, fix, sci, gen, gen, gen, hex, gen
            return ((lut >> ((sanitized_type_char & 0b111) << 2)) & 0b1111);
        }
    }
}
