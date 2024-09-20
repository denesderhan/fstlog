//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/ut_cast.hpp>

namespace fstlog {
	enum class logfield : unsigned char {
		Invalid = 0,
		Message = 1,
		Severity = 2,
		Timestamp = 3,
		Logger = 4,
		Thread = 5,
		File = 6,
		Line = 7,
		Function = 8,
		Channel = 9,
		Policy = 10,
		Args = 11
	};

	inline constexpr auto logfield_last{ logfield::Args };
}
