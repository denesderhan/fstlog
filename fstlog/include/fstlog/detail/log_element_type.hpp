//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/types.hpp>

namespace fstlog {
	//used bits 7-4
	enum class log_element_type : log_element_ut {
		Unloggable = 0,
		Char =  1 << 4,
		Bool = 2 << 4,
		String = 3 << 4,
		Float = 4 << 4,
		Integral = 5 << 4,
		Aggregate = 6 << 4,
		Hash = 7 << 4,
		Pointer = 8 << 4,
		Stringptr = 9 << 4,
		Smallstring = 10 << 4
	};
	inline constexpr log_element_ut log_element_type_bitmask = 0xf0;
}
