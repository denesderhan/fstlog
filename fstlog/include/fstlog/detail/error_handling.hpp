//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/noexceptions.hpp>

#ifdef FSTLOG_NOEXCEPTIONS
#include <cassert>
namespace fstlog {
#ifdef FSTLOG_DEBUG
	inline void handle_error(const char* error) noexcept {
		assert(error == nullptr);
	}
#else
	inline void handle_error(const char* error) noexcept {}
#endif
	inline void error_if(bool condition, const char* message) noexcept {
		assert(!condition && message);
	}
}
#else
#include <fstlog/detail/fstlog_ex.hpp>
namespace fstlog {
	inline void handle_error(const char* error) {
		if (error != nullptr)
			throw fstlog_ex(error);
	}
	inline void error_if(bool condition, const char* message) {
		if (condition) throw fstlog_ex(message);
	}
}
#endif
