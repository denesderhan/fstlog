//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <type_traits>
namespace fstlog {
	template<typename T>
	using rm_cvref_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
}
