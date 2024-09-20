//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/output/output.hpp>

#include <stdio.h>

#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/fstlog_allocator.hpp>

namespace fstlog {
	// non thread safe, opening/closing FILE* is callers responsibility
	// FILE* must be opened in binary mode
    FSTLOG_API const char* output_cstream(
		output& out,
        FILE* file, 
        fstlog_allocator const& allocator) noexcept;
	// non thread safe, opening/closing FILE* is callers responsibility
	// FILE* must be opened in binary mode
	inline output output_cstream(
		FILE* file,
		fstlog_allocator const& allocator = {}) noexcept(noexcept(handle_error("")))
	{
		output out;
		[[maybe_unused]] const auto error = output_cstream(out, file, allocator);
		handle_error(error);
		return out;
	}
}

