//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstring>
#include <limits>
#include <type_traits>
#pragma intrinsic(memcpy)

namespace fstlog {
	namespace detail {
		template <typename T>
		inline std::make_unsigned_t<T> abs_unsigned(T data) noexcept {
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

		static inline void to_upper_case(char* begin, char* end) noexcept {
			while (begin < end) {
				if (*begin >= 'a') *begin -= ('a' - 'A');
				begin++;
			}
		}

		static inline void write_sign(
			unsigned char sign_fmt,
			bool num_negative,
			unsigned char*& pos) noexcept
		{
			FSTLOG_ASSERT(sign_fmt == '-' || sign_fmt == '+'
				|| sign_fmt == ' ' && "Invalid sign format!");
			if (sign_fmt == '-') {
				if (!num_negative) sign_fmt = 0;
			}
			// sign_char == '+' || sign_char == ' '
			else {
				if (num_negative) sign_fmt = '-';
			}
			if (sign_fmt != 0) *pos++ = sign_fmt;
		}

		inline static int handle_prefix(
			unsigned char base_char,
			bool write_prefix,
			unsigned char*& pos)
		{
			int base = 10;
			switch (base_char) {
			case 'd': break;
			case 0:	break;
			case 'x':
				base = 16;
				if (write_prefix) {
					*pos++ = '0';
					*pos++ = 'x';
				}
				break;
			case 'X':
				base = 16;
				if (write_prefix) {
					*pos++ = '0';
					*pos++ = 'X';
				}
				break;
			case 'b':
				base = 2;
				if (write_prefix) {
					*pos++ = '0';
					*pos++ = 'b';
				}
				break;
			case 'B':
				base = 2;
				if (write_prefix) {
					*pos++ = '0';
					*pos++ = 'B';
				}
				break;
			case 'o':
				base = 8;
				if (write_prefix) {
					*pos++ = '0';
				}
				break;
			default:
				break;
			}
			return base;
		}
	}
}
