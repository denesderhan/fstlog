//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/types.hpp>
namespace fstlog {
	enum class level : level_type {
		None = 0,
		Fatal = 1,
		Error = 2,
		Warn = 3,
		Info = 4,
		Debug = 5,
		Trace = 6,
		All = 7
	};
}
