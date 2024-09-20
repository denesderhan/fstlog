//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/types.hpp>

namespace fstlog {
	enum class log_metaargs : log_call_flag {
		None = 0,
		Logger = 1,
		Thread = 2,
		File = 4,
		Line = 8,
		Function = 16,
		All = 255
	};
}
