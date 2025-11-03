//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/sink/sink.hpp>

#include <fstlog/compatible.hpp>
#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/memory_resource.hpp>

namespace fstlog {
    FSTLOG_API error_code sink_null(
        sink& out,
        memory_resource* resource) noexcept;

    inline sink sink_null(
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(handle_error(error_code::none)))
    {
        sink out;
        error_code error{ error_code::none };
        if (!compatible()) error = error_code::incomp_api;
        else if (!memory_resource_identical()) error = error_code::mem_res_bad;
        else error = sink_null(out, resource);
        handle_error(error);
        return out;
    }
}

