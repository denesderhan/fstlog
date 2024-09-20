//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/output/output.hpp>

#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/fstlog_allocator.hpp>

namespace fstlog {
	FSTLOG_API const char* output_null(
		output& out,
		fstlog_allocator const& allocator) noexcept;
	inline output output_null(
		fstlog_allocator const& allocator = {}) noexcept(noexcept(handle_error("")))
	{
		output out;
		[[maybe_unused]] const auto error = output_null(out, allocator);
		handle_error(error);
		return out;
	}
}
