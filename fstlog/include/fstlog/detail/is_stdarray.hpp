//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstddef>
#include <type_traits>

#include <fstlog/detail/rm_cvref_t.hpp>

namespace fstlog {
	namespace {
		template<class T>
		struct is_stdarray_impl : std::false_type {};

		template<class T, std::size_t N>
		struct is_stdarray_impl<std::array<T, N>> : std::true_type {};
	}

	template <typename T>
	struct is_stdarray {
		static constexpr bool value{ is_stdarray_impl<rm_cvref_t<T>>::value };
	};

	template <typename T>
	constexpr bool is_stdarray_v = is_stdarray<T>::value;
}
