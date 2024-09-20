//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#ifndef FSTLOG_SINK_FLUSHINTERVAL
#define FSTLOG_SINK_FLUSHINTERVAL 0
#endif
#include <chrono>
namespace fstlog {
	namespace config {
		inline constexpr std::chrono::milliseconds default_sink_flush_interval{ FSTLOG_SINK_FLUSHINTERVAL };
	}
}
