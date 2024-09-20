//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/types.hpp>
namespace fstlog {
	//bits: 7-6
	//not flags (2 bit, 3 value)
	enum class log_policy : log_call_flag {
		Guaranteed = 0x00,
		NonGuaranteed = 0x40,
		LowLatency = 0x80
	};
}
