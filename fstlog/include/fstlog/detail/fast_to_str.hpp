//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <limits>
#include <string_view>
#include <type_traits>

namespace fstlog {
	template<typename T, std::size_t buff_size = sizeof(T) * 2 + 2>
	inline std::string_view to_hex(T num, char(&buffer)[buff_size]) noexcept {
		static_assert(std::numeric_limits<unsigned char>::digits == 8);
		static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "Wrong type!");
		static_assert(buff_size >= sizeof(T) * 2 + 2, "Buffer too small!");
		auto c_ptr{ buffer + buff_size };
		std::size_t length{ 0 };
		const char digits[]{ "0123456789abcdef" };
		do {
			*--c_ptr = digits[num & 0xf];
			length++;
			num >>= 4;
		} while (num != 0);
		*--c_ptr = 'x';
		*--c_ptr = '0';
		length += 2;
		return std::string_view{ c_ptr, length };
	}

	template<typename T, 
		std::size_t buff_size = 
			(sizeof(T) * std::numeric_limits<unsigned char>::digits * 302) / 1000 + 1>
	inline std::string_view to_dec(T num, char(&buffer)[buff_size]) noexcept {
		static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "Wrong type!");
		static_assert(buff_size >= 
			(sizeof(T) * std::numeric_limits<unsigned char>::digits * 302) / 1000 + 1, 
			"Buffer too small!");
		auto c_ptr{ buffer + buff_size };
		const char digits2[]{
			"0001020304050607080910111213141516171819"
			"2021222324252627282930313233343536373839"
			"4041424344454647484950515253545556575859"
			"6061626364656667686970717273747576777879"
			"8081828384858687888990919293949596979899" };

		std::size_t length{ 0 };
		do {
			T div_num = num / 100;
			int ind = static_cast<int>(num - (div_num * 100)) * 2;
			*(--c_ptr) = digits2[ind + 1];
			*(--c_ptr) = digits2[ind];
			num = div_num;
			length += 2;
		} while (num != 0);
		if (*c_ptr == '0') {
			c_ptr++;
			length--;
		}
		return std::string_view{ c_ptr, length };
	}

}
