//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/compatible.hpp>
#include <fstlog/detail/api_def.hpp>
#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/level.hpp>
#include <fstlog/detail/memory_resource.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
    class filter_impl;
    class filter {
    public:
        FSTLOG_API filter() noexcept(noexcept(fstlog::get_default_resource())
            && noexcept(handle_error(error_code::none)))
            :filter(fstlog::get_default_resource()) {
        }

        explicit FSTLOG_API filter(memory_resource* resource) noexcept(
            noexcept(handle_error(error_code::none)))
        {
            error_code error{ error_code::none };
            if (!compatible()) error = error_code::incomp_api;
            else if (!memory_resource_identical()) error = error_code::mem_res_bad;
            else error = init(resource);
            handle_error(error);
        }

        FSTLOG_API filter(
            level level,
            channel_type channel,
            memory_resource* resource = fstlog::get_default_resource()) noexcept(
                noexcept(handle_error(error_code::none)))
        {
            error_code error{ error_code::none };
            if (!compatible()) error = error_code::incomp_api;
            else if (!memory_resource_identical()) error = error_code::mem_res_bad;
            else error = init(level, channel, resource);
            handle_error(error);
        }

        FSTLOG_API filter(
            level level,
            channel_type first_channel,
            channel_type last_channel,
            memory_resource* resource = fstlog::get_default_resource()) noexcept(
                noexcept(handle_error(error_code::none)))
        {
            error_code error{ error_code::none };
            if (!compatible()) error = error_code::incomp_api;
            else if (!memory_resource_identical()) error = error_code::mem_res_bad;
            else error = 
                init(level, first_channel, last_channel, resource);
            handle_error(error);
        }

        FSTLOG_API filter(const filter& other) noexcept(
            noexcept(handle_error(error_code::none))) 
        {
            error_code error{ error_code::none };
            if (!compatible()) error = error_code::incomp_api;
            else if (!memory_resource_identical()) error = error_code::mem_res_bad;
            else error = init(other);
            handle_error(error);
        }

        FSTLOG_API filter& operator=(const filter& other) noexcept;
        FSTLOG_API filter(filter&& other) noexcept;
        FSTLOG_API filter& operator=(filter&& other) noexcept;
        FSTLOG_API bool operator==(const filter& other) const noexcept;
        FSTLOG_API bool operator!=(const filter& other) const noexcept;
        FSTLOG_API ~filter() noexcept;

        FSTLOG_API bool good() const noexcept;
        
        FSTLOG_API void add_level(level level) noexcept;
        FSTLOG_API void add_level(level first, level last) noexcept;
        FSTLOG_API void add_channel(channel_type channel) noexcept;
        FSTLOG_API void add_channel(channel_type first, channel_type last) noexcept;
        FSTLOG_API bool filter_msg(level level, channel_type channel) const noexcept;

        FSTLOG_API filter(filter_impl* pimpl) noexcept;
        
        FSTLOG_TEST_API filter_impl* pimpl() const noexcept;

    private:
        FSTLOG_API error_code init(memory_resource* resource) noexcept;
        FSTLOG_API error_code init(
            level level,
            channel_type channel,
            memory_resource* resource) noexcept;
        FSTLOG_API error_code init(
            level level,
            channel_type first_channel,
            channel_type last_channel,
            memory_resource* resource) noexcept;
        FSTLOG_API error_code init(const filter& other) noexcept;
        
        filter_impl* pimpl_{ nullptr };
    };
}
