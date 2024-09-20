//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <type_traits>
#include <tuple>

#include <fstlog/detail/rm_cvref_t.hpp>

namespace fstlog {
	namespace {
		template <typename>
		struct is_tuple_impl
			: std::false_type {};

		template <typename ...Ts>
		struct is_tuple_impl<std::tuple<Ts...>>
			: std::true_type {};

		template <typename T, typename U>
		struct is_tuple_impl<std::pair<T, U>>
			: std::true_type {};
	}

	template <typename T>
	struct is_tuple  {
		static constexpr bool value{is_tuple_impl<rm_cvref_t<T>>::value};
	};

	template <typename T>
	constexpr bool is_tuple_v = is_tuple<T>::value;
}
