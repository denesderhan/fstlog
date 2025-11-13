//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/sink/sink.hpp>

#include <chrono>

#include <fstlog/compatible.hpp>
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
        error_code error{ error_code::none };
        if (!compatible()) error = error_code::incomp_api;
        else if (!memory_resource_identical()) error = error_code::mem_res_bad;
        else error = sink_unsort(
            out, 
            std::move(formatter), 
            std::move(output),
            resource);
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
        error_code error{ error_code::none };
        if (!compatible()) error = error_code::incomp_api;
        else if (!memory_resource_identical()) error = error_code::mem_res_bad;
        else error = sink_unsort(
            out, 
            std::move(formatter),
            std::move(output),
            std::move(filter),
            resource);
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
        error_code error{ error_code::none };
        if (!compatible()) error = error_code::incomp_api;
        else if (!memory_resource_identical()) error = error_code::mem_res_bad;
        else error = sink_unsort(
            out, 
            std::move(formatter),
            std::move(output),
            std::move(filter),
            flush_interval,
            resource);
        handle_error(error);
        return out;
    }
}
