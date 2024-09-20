//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/output/output.hpp>

#include <cstdint>

#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/fstlog_allocator.hpp>

namespace fstlog {
	FSTLOG_API const char* output_file(
		output& out,
		const char* file_path,
		fstlog_allocator const& allocator = {}) noexcept;
	inline output output_file(
		const char* file_path,
		fstlog_allocator const& allocator = {}) noexcept(noexcept(handle_error("")))
	{
		output out;
		[[maybe_unused]] const auto error = output_file(out, file_path, allocator);
		handle_error(error);
		return out;
	}

	FSTLOG_API const char* output_file(
		output& out,
		const char* file_path,
		bool truncate,
		fstlog_allocator const& allocator = {}) noexcept;
	inline output output_file(
		const char* file_path,
		bool truncate,
		fstlog_allocator const& allocator = {}) noexcept(noexcept(handle_error("")))
	{
		output out;
		[[maybe_unused]] const auto error = output_file(out, file_path, truncate, allocator);
		handle_error(error);
		return out;
	}
	FSTLOG_API const char* output_file(
		output& out,
		const char* file_path,
		bool truncate,
		std::uint32_t buffer_size,
		fstlog_allocator const& allocator = {}) noexcept;
	inline output output_file(
		const char* file_path,
		bool truncate,
		std::uint32_t buffer_size,
		fstlog_allocator const& allocator = {}) noexcept(noexcept(handle_error("")))
	{
		output out;
		[[maybe_unused]] const auto error = output_file(out, file_path, truncate, buffer_size, allocator);
		handle_error(error);
		return out;
	}
}
