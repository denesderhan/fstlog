//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <chrono>
#include <cstdint>
#include <cstddef>
#include <string_view>

#include <fstlog/detail/api_def.hpp>
#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/fstlog_allocator.hpp>
#include <fstlog/detail/log_buffer.hpp>
#include <fstlog/sink/sink.hpp>

namespace fstlog {
	class core_impl;
    class core {
    public:
        using allocator_type = fstlog_allocator;

		FSTLOG_API core() noexcept(
			noexcept(allocator_type())
			&& noexcept(handle_error("")))
			: core(allocator_type{}) {}
		FSTLOG_API explicit core(allocator_type const& allocator)  noexcept(
			noexcept(handle_error("")))
		{
			[[maybe_unused]] const auto error = init(allocator);
			handle_error(error);
		}
		FSTLOG_API core(
			std::string_view name, 
			allocator_type const& allocator = {}) noexcept(noexcept(handle_error("")))
		{
			[[maybe_unused]] const auto error = init(name, allocator);
			handle_error(error);
		}
		FSTLOG_API ~core() noexcept;
		FSTLOG_API core(const core& other) noexcept;
		FSTLOG_API core& operator=(const core& other) noexcept;
		FSTLOG_API core(core&& other) noexcept;
		FSTLOG_API core& operator=(core&& other) noexcept;

		FSTLOG_API const char* init(allocator_type const& allocator = {}) noexcept;
		FSTLOG_API const char* init(
			std::string_view name,
			allocator_type const& allocator = {}) noexcept;

		FSTLOG_API bool start() noexcept;
		FSTLOG_API bool stop() noexcept;
		FSTLOG_API bool restart() noexcept;
		FSTLOG_API bool running() const noexcept;
		FSTLOG_API std::chrono::milliseconds poll_interval(std::chrono::milliseconds poll_interval) noexcept;
		FSTLOG_API std::chrono::milliseconds poll_interval() const noexcept;
		FSTLOG_API bool add_sink(sink sink) noexcept;
		FSTLOG_API bool release_sink(sink& sink) noexcept ;
		FSTLOG_API void flush() const noexcept;
		FSTLOG_API std::string_view name() const noexcept;
		FSTLOG_API bool good() const noexcept;
		FSTLOG_API std::string_view version() const noexcept;
		FSTLOG_API int version_major() const noexcept;
		FSTLOG_API int version_minor() const noexcept;
		FSTLOG_API int version_patch() const noexcept;
		FSTLOG_API std::uintmax_t id() const noexcept;

		FSTLOG_API void notify_data_ready() const noexcept;
		FSTLOG_API log_buffer get_buffer(std::uint32_t buffer_size) noexcept;
		FSTLOG_API explicit core(core_impl* pimpl)  noexcept;
		
		core_impl* pimpl() const noexcept;
	private:
		core_impl* pimpl_{ nullptr };
    };
}
