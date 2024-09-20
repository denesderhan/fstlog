//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>
#include <limits>
#include <type_traits>

namespace fstlog {
	namespace detail {
		template<typename T>
		inline constexpr T nearest_pow2(T num) noexcept {
			static_assert(std::is_unsigned_v<T>, "T can only be unsigned!");
			if (num == 0) return 1;
			constexpr T max_pow2 = 
				T(1) << ((sizeof(T) * std::numeric_limits<unsigned char>::digits) - 1);
			if (num >= max_pow2) return max_pow2;
			T lower = max_pow2;
			while (lower > num) {
				lower = lower >> 1;
			}
			const T upper = lower << 1;
			if (num - lower < upper - num) {
				return lower;
			}
			else {
				return upper;
			}
		}
	}
}
