//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstddef>

#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/array_copy.hpp>

namespace fstlog {
	template <typename T, std::size_t... Sizes>
	inline constexpr auto array_concat(const std::array<T, Sizes>... arrays) noexcept {
		std::array<T, (Sizes + ...)> result{};
		std::size_t index{0};
		((array_copy(result, arrays, index), index += Sizes), ...);
		return result;
	}

}
