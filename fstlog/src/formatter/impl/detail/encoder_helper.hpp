//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstring>
#include <limits>
#include <type_traits>
#pragma intrinsic(memcpy)

#include <fstlog/detail/constants.hpp>

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

		inline static int get_int_base(unsigned char base_char) noexcept {
			//[10(/0), 2(B,b), 10(d), 10(?), 16(X,x), 10(?), 10(?), 8(o)]
			alignas(constants::cache_ls_nosharing) const std::array<int, 8> base_table{
				10, 2, 10, 10, 16, 10, 10, 8 };
			return base_table[(base_char >> 1) & 0b111];
		}

		inline void write_prefix(int base, unsigned char base_char, unsigned char* &pos) noexcept {
			if (base == 10) return;
			*pos++ = '0';
			if (base == 8) return;
			*pos++ = base_char;
		}
	}
}
