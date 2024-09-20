//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstddef>

#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
	template <typename T, std::size_t N1, std::size_t N2 >
	inline constexpr void array_copy(
		std::array<T, N1>& to_array, 
		const  std::array<T, N2>& from_array, 
		std::size_t index) noexcept 
	{
		FSTLOG_ASSERT(index < N1 && "Invalid index value!");
		FSTLOG_ASSERT(index + N2 <= N1 && "Index out of bounds!");
		for (std::size_t i = 0; i < N2; i++) {
			to_array[index + i] = from_array[i];
		}
	}
}
