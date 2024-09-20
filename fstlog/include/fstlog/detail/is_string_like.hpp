//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <string_view>
#include <type_traits>

#include <fstlog/detail/rm_cvref_t.hpp>

namespace fstlog {
	template <typename T, typename = void>
	struct is_string_like
		: std::false_type {};

	template <typename T>
	struct is_string_like<T, std::enable_if_t<
		!std::is_pointer_v<std::remove_reference_t<T>> && (
		std::is_convertible_v<rm_cvref_t<T>, std::string_view>
#ifdef __cpp_char8_t		
		|| std::is_convertible_v<rm_cvref_t<T>, std::u8string_view>
#endif		
		|| std::is_convertible_v<rm_cvref_t<T>, std::u16string_view>
		|| std::is_convertible_v<rm_cvref_t<T>, std::u32string_view>)
		>>
		: std::true_type{};

	template <typename T>
	constexpr bool is_string_like_v = is_string_like<T>::value;
}
