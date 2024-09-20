//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>
#include <limits>
#include <type_traits>

#include <fstlog/detail/is_pow2.hpp>

namespace fstlog {
	template <std::uintmax_t padd_to, typename T>
	constexpr T padded_size(T num) noexcept {
		static_assert(std::is_unsigned_v<T>);
		if constexpr (padd_to == 1) {
			return num;
		}
		else {
			static_assert(is_pow2(padd_to), "padd_to must be power of two!");
			static_assert(padd_to <= (std::numeric_limits<T>::max)(), "padd_to must be less than T max");
			constexpr T padding = padd_to;
			constexpr T mask = ~(padding - 1);
			T comp = num & mask;
			if (comp != num) {
				comp += padding;
				if (comp > num) return comp;
			}
			return num;
		}
	}
}
