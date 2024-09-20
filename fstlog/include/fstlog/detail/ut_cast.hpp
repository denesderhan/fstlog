//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <type_traits>
namespace fstlog {
	//casts the enum type to its underlying type.
	template <typename T>
	constexpr auto ut_cast(T const& x) noexcept 
		-> std::enable_if_t<std::is_enum_v<T>, std::underlying_type_t<T>> 
	{
		return static_cast<std::underlying_type_t<T>>(x);
	}
}
