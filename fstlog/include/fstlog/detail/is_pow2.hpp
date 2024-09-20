//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <type_traits>

namespace fstlog {
	template<typename T>
	inline constexpr bool is_pow2(T num) noexcept {
		static_assert(std::is_unsigned_v<T>, "Only for unsigned!");
		return num && ((num & (num - 1)) == 0);
	}
}
