//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <chrono>
#include <cstdint>
#include <cstddef>
#include <string_view>

#include <fstlog/compatible.hpp>
#include <fstlog/detail/api_def.hpp>
#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/memory_resource.hpp>
#include <fstlog/detail/log_buffer.hpp>
#include <fstlog/sink/sink.hpp>

namespace fstlog {
    class core_impl;
    class core {
    public:
        FSTLOG_API core() noexcept(noexcept(get_default_resource()) 
            && noexcept(handle_error(error_code::none)))
            :core(get_default_resource()) {}

        FSTLOG_API explicit core(memory_resource* resource) noexcept(
            noexcept(handle_error(error_code::none)))
        {
            error_code error{ error_code::none };
            if (!compatible()) error = error_code::incomp_api;
            else if (!memory_resource_identical()) error = error_code::mem_res_bad;
            else error = init(resource);
            handle_error(error);
        }
        FSTLOG_API core(
            std::string_view name, 
            memory_resource* resource = fstlog::get_default_resource()) noexcept(
                noexcept(handle_error(error_code::none)))
        {
            error_code error{ error_code::none };
            if (!compatible()) error = error_code::incomp_api;
            else if (!memory_resource_identical()) error = error_code::mem_res_bad;
            else error = init(name, resource);
            handle_error(error);
        }
        FSTLOG_API ~core() noexcept;
        FSTLOG_API core(const core& other) noexcept;
        FSTLOG_API core& operator=(const core& other) noexcept;
        FSTLOG_API core(core&& other) noexcept;
        FSTLOG_API core& operator=(core&& other) noexcept;
        FSTLOG_API bool operator==(const core& other) const noexcept;
        FSTLOG_API bool operator!=(const core& other) const noexcept;

        FSTLOG_API bool start() noexcept;
        FSTLOG_API bool stop() noexcept;
        FSTLOG_API bool restart() noexcept;
        FSTLOG_API bool running() const noexcept;
        FSTLOG_API std::chrono::milliseconds poll_interval(std::chrono::milliseconds poll_interval) noexcept;
        FSTLOG_API std::chrono::milliseconds poll_interval() const noexcept;
        FSTLOG_API bool add_sink(sink sink) noexcept;
        FSTLOG_API bool release_sink(sink& sink) noexcept;
        FSTLOG_API void flush() const noexcept;
        FSTLOG_API std::string_view name() const noexcept;
        FSTLOG_API bool good() const noexcept;
        FSTLOG_API std::uintmax_t id() const noexcept;
        FSTLOG_API static std::size_t limit() noexcept;

        FSTLOG_API void detail_notify_data_ready() const noexcept;
        FSTLOG_API log_buffer detail_get_buffer(std::uint32_t buffer_size) noexcept;
        FSTLOG_API log_buffer& detail_tls_buffer() noexcept;
        FSTLOG_API explicit core(std::nullptr_t) noexcept;
        
        FSTLOG_API core_impl* pimpl() const noexcept;
    private:
        FSTLOG_API explicit core(core_impl* pimpl) noexcept;
        FSTLOG_API error_code init(
            memory_resource* resource) noexcept;
        FSTLOG_API error_code init(
            std::string_view name,
            memory_resource* resource) noexcept;
        
        core_impl* pimpl_{ nullptr };
    };
}
