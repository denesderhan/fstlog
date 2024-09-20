//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/output/output.hpp>

#include <memory>
#include <mutex>
#include <ostream>

#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/fstlog_allocator.hpp>

namespace fstlog {
	// thread safe, ofstream must be opened in binary mode
	FSTLOG_API const char* output_stream_mt(
		output& out,
		std::shared_ptr<std::ostream> stream,
		std::shared_ptr<std::mutex> mutex,
		fstlog_allocator const& allocator = {}) noexcept;
	// thread safe, ofstream must be opened in binary mode
	inline output output_stream_mt(
		std::shared_ptr<std::ostream> stream,
		std::shared_ptr<std::mutex> mutex,
		fstlog_allocator const& allocator = {}) noexcept(noexcept(handle_error("")))
	{
		output out;
		[[maybe_unused]] const auto error = output_stream_mt(out, stream, mutex, allocator);
		handle_error(error);
		return out;
	}
}
