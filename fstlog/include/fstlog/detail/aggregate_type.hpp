//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/types.hpp>
namespace fstlog {
	//used bits 3-0
	enum class aggregate_type : log_element_ut {
		List =  0,
		Tuple = 1
	};
}
