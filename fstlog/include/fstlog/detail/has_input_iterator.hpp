//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <type_traits>
#include <iterator>

#include <fstlog/detail/rm_cvref_t.hpp>

namespace fstlog {
	template <typename T, typename = void>
	struct has_input_iterator
		: std::false_type {};

	template <typename T>
	struct has_input_iterator<T, std::enable_if_t<
		std::is_base_of_v<std::input_iterator_tag,
		typename std::iterator_traits<
		decltype(std::declval<rm_cvref_t<T>>().begin())
		>::iterator_category>>>
		: std::true_type {};

	template <typename T>
	constexpr bool has_input_iterator_v = has_input_iterator<T>::value;
}
