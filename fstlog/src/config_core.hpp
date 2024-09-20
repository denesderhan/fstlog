//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#ifndef FSTLOG_POLLINTERVAL
#define FSTLOG_POLLINTERVAL 0
#endif
#include <chrono>

namespace fstlog {
	namespace config {
		//Default core background thread full flush period in milliseconds.
		inline constexpr std::chrono::milliseconds default_polling_interval{ FSTLOG_POLLINTERVAL };
	}
}
