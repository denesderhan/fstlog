//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/version.hpp>
#include <fstlog/core.hpp>

namespace fstlog {
	inline bool compatible(core const& core_) noexcept {
		if (FSTLOG_VERSION_MAJOR != 0) {
			return (core_.version_major() == FSTLOG_VERSION_MAJOR &&
				core_.version_minor() >= FSTLOG_VERSION_MINOR);
		}
		else {
			return core_.version_minor() == FSTLOG_VERSION_MINOR &&
				core_.version_patch() == FSTLOG_VERSION_PATCH;
		}
	}
}
