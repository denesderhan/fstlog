//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/sink/sink.hpp>

#include <chrono>

#include <fstlog/detail/api_def.hpp>
#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/memory_resource.hpp>
#include <fstlog/filter/filter.hpp>
#include <fstlog/formatter/formatter.hpp>
#include <fstlog/output/output.hpp>

namespace fstlog {
    FSTLOG_API error_code sink_unsort(
        sink& out,
        formatter formatter, 
        output output, 
        memory_resource* resource) noexcept;
    FSTLOG_API error_code sink_unsort(
        sink& out,
        formatter formatter, 
        output output,
        filter filter,
        memory_resource* resource) noexcept;
    FSTLOG_API error_code sink_unsort(
        sink& out,
        formatter formatter, 
        output output,
        filter filter,
        std::chrono::milliseconds flush_interval, 
        memory_resource* resource) noexcept;
    
    inline sink sink_unsort(
        formatter formatter,
        output output,
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(handle_error(error_code::none)))
    {
        sink out;
        const auto error = 
            sink_unsort(out, formatter, output, resource);
        handle_error(error);
        return out;
    }
    inline sink sink_unsort(
        formatter formatter,
        output output,
        filter filter,
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(handle_error(error_code::none)))
    {
        sink out;
        const auto error = 
            sink_unsort(out, formatter, output, filter, resource);
        handle_error(error);
        return out;
    }
    inline sink sink_unsort(
        formatter formatter,
        output output,
        filter filter,
        std::chrono::milliseconds flush_interval,
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(handle_error(error_code::none)))
    {
        sink out;
        const auto error =
            sink_unsort(out, formatter, output, filter, flush_interval, resource);
        handle_error(error);
        return out;
    }
}
