//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/sink/sink.hpp>

#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/fstlog_allocator.hpp>

namespace fstlog {
    FSTLOG_API const char* sink_null(
		sink& out,
		fstlog_allocator const& allocator) noexcept;
	inline sink sink_null(
		fstlog_allocator const& allocator = {}) noexcept(noexcept(handle_error("")))
	{
		sink out;
		[[maybe_unused]] const auto error = sink_null(out, allocator);
		handle_error(error);
		return out;
	}
}

