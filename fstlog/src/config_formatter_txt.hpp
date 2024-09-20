//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <string_view>

namespace fstlog {
	namespace config {
		//Default format string for text type formatters.
		inline constexpr std::string_view default_format_string{ 
			"{timestamp:.6%Y-%m-%d %H:%M:%S %z} {severity} {file}:{line} {message}" };
	}
}
