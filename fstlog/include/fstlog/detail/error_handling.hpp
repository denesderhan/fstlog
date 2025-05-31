//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/noexceptions.hpp>

#ifdef FSTLOG_NOEXCEPTIONS
#ifndef NDEBUG
#include <cstdlib>
#include <cstdio>

namespace fstlog {
	inline void handle_error(error_code error) noexcept {
		if (error != error_code::none) {
			std::fprintf(stderr, "Assertion failed: %s\n", error_message(error));
			std::abort();
		}
	}

	inline void error_if(bool condition, error_code error) noexcept {
		if (condition) {
			std::fprintf(stderr, "Assertion failed: %s\n", error_message(error));
			std::abort();
		}
	}
}
#else
namespace fstlog {
	inline void handle_error([[maybe_unused]] error_code error) noexcept {}
	inline void error_if([[maybe_unused]] bool condition, [[maybe_unused]] error_code error) noexcept {}
}
#endif
#else
#include <fstlog/detail/fstlog_ex.hpp>

namespace fstlog {
	inline void handle_error(error_code error) {
		if (error != error_code::none) {
			throw fstlog_ex(error_message(error));
		}
	}
	inline void error_if(bool condition, error_code error) {
		if (condition) {
			throw fstlog_ex(error_message(error));
		}
	}
}
#endif
