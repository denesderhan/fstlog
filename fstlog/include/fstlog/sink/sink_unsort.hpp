//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/sink/sink.hpp>

#include <chrono>

#include <fstlog/detail/api_def.hpp>
#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/fstlog_allocator.hpp>
#include <fstlog/filter/filter.hpp>
#include <fstlog/formatter/formatter.hpp>
#include <fstlog/output/output.hpp>

namespace fstlog {
    FSTLOG_API const char* sink_unsort(
		sink& out,
        formatter formatter, 
        output output, 
        fstlog_allocator const& allocator = {}) noexcept;
    FSTLOG_API const char* sink_unsort(
		sink& out,
		formatter formatter, 
		output output,
        filter filter,
        fstlog_allocator const& allocator = {}) noexcept;
    FSTLOG_API const char* sink_unsort(
		sink& out,
		formatter formatter, 
		output output,
        filter filter,
        std::chrono::milliseconds flush_interval, 
        fstlog_allocator const& allocator = {}) noexcept;
	
	inline sink sink_unsort(
		formatter formatter,
		output output,
		fstlog_allocator const& allocator = {}) noexcept(noexcept(handle_error("")))
	{
		sink out;
		[[maybe_unused]] const auto error = 
			sink_unsort(out, formatter, output, allocator);
		handle_error(error);
		return out;
	}
	inline sink sink_unsort(
		formatter formatter,
		output output,
		filter filter,
		fstlog_allocator const& allocator = {})  noexcept(noexcept(handle_error("")))
	{
		sink out;
		[[maybe_unused]] const auto error = 
			sink_unsort(out, formatter, output, filter, allocator);
		handle_error(error);
		return out;
	}
	inline sink sink_unsort(
		formatter formatter,
		output output,
		filter filter,
		std::chrono::milliseconds flush_interval,
		fstlog_allocator const& allocator = {})  noexcept(noexcept(handle_error("")))
	{
		sink out;
		[[maybe_unused]] const auto error =
			sink_unsort(out, formatter, output, filter, flush_interval, allocator);
		handle_error(error);
		return out;
	}
}
