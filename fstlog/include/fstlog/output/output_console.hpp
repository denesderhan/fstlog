//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/output/output_cstream.hpp>

namespace fstlog {
    inline output output_console(
		fstlog_allocator const& allocator = {}) noexcept
	{
		return output_cstream(stdout, allocator);
	}
	inline output output_stderr(
		fstlog_allocator const& allocator = {}) noexcept
	{
		return output_cstream(stderr, allocator);
	}
}
