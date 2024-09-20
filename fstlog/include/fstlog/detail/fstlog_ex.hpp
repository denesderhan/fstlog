//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <stdexcept>

namespace fstlog {
	class fstlog_ex : public std::runtime_error {
	public:
		fstlog_ex(const char* message) noexcept 
			: runtime_error(message) {}
	};
}
