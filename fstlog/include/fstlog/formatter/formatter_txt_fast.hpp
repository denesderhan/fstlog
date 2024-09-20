//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/formatter/formatter.hpp>

#include <string_view>

#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/fstlog_allocator.hpp>

namespace fstlog {
	FSTLOG_API const char* formatter_txt_fast(
		formatter& out,
		fstlog_allocator const& allocator = {}) noexcept;
	FSTLOG_API const char* formatter_txt_fast(
		formatter& out,
		std::string_view format_string,
		fstlog_allocator const& allocator = {}) noexcept;
	inline formatter formatter_txt_fast(
		fstlog_allocator const& allocator = {}) noexcept(noexcept(handle_error("")))
	{
		formatter out;
		[[maybe_unused]] const auto error = formatter_txt_fast(out, allocator);
		handle_error(error);
		return out;
	}
	inline formatter formatter_txt_fast(
		std::string_view format_string,
		fstlog_allocator const& allocator = {}) noexcept(noexcept(handle_error("")))
	{
		formatter out;
		[[maybe_unused]] const auto error = formatter_txt_fast(out, format_string, allocator);
		handle_error(error);
		return out;
	}
#ifdef __cpp_char8_t
	inline formatter formatter_txt_fast(
		std::u8string_view format_string,
		fstlog_allocator const& allocator = {}) noexcept(noexcept(handle_error("")))
	{
		return formatter_txt_fast(
			std::string_view{ reinterpret_cast<const char*>(format_string.data()), format_string.size() },
			allocator);
	}
#endif
}
