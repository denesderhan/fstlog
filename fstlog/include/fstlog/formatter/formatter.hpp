//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/fstlog_allocator.hpp>
#include <fstlog/detail/api_def.hpp>

namespace fstlog {
	class formatter_interface;
	class formatter {
	public:
		FSTLOG_API formatter() noexcept;
		FSTLOG_API ~formatter() noexcept;
		FSTLOG_API formatter(const formatter& other) noexcept;
		FSTLOG_API formatter& operator=(const formatter& other) noexcept;
		FSTLOG_API formatter(formatter&& other) noexcept;
		FSTLOG_API formatter& operator=(formatter&& other) noexcept;
		FSTLOG_API formatter clone() const noexcept(noexcept(handle_error(""))) {
			formatter out{};
			[[maybe_unused]] const auto error = clone(out);
			handle_error(error);
			return out;
		}
		FSTLOG_API formatter clone(
			fstlog_allocator const& allocator) const noexcept(noexcept(handle_error(""))) 
		{
			formatter out{};
			[[maybe_unused]] const auto error = clone(out, allocator);
			handle_error(error);
			return out;
		}
		
		FSTLOG_API bool good() const noexcept;
		FSTLOG_API const char* clone(formatter& out) const noexcept;
		FSTLOG_API const char* clone(formatter& out, fstlog_allocator const& allocator) const noexcept;
		
		explicit formatter(formatter_interface* pimpl) noexcept;
		formatter_interface* pimpl() const noexcept;
	private:
		formatter_interface* pimpl_{ nullptr };
	};
}
