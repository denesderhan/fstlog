//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <type_traits>

#include <fstlog/detail/rm_cvref_t.hpp>

namespace fstlog {
	template <typename T, typename = void>
	struct is_char_type
		: std::false_type {};

	template <typename T>
	struct is_char_type<T, std::enable_if_t<
		std::is_same_v<rm_cvref_t<T>, char>
#ifdef __cpp_char8_t		
		|| std::is_same_v<rm_cvref_t<T>, char8_t>
#endif		
		|| std::is_same_v<rm_cvref_t<T>, char16_t>
		|| std::is_same_v<rm_cvref_t<T>, char32_t>
		>>
		: std::true_type{};

	template <typename T>
	constexpr bool is_char_type_v = is_char_type<T>::value;
}
