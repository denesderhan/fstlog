//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/types.hpp>
namespace fstlog {
	//used bits 3-0
	enum class char_type : log_element_ut {
		Char = 0,
		Char8 =  1,
		Char16 = 2,
		Char32 = 3
	};
}
