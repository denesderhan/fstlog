//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/output/output.hpp>

#include <stdio.h>

#include <fstlog/compatible.hpp>
#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/memory_resource.hpp>

namespace fstlog {
    // non thread safe, opening/closing FILE* is caller's responsibility
    FSTLOG_API error_code output_cstream(
        output& out,
        FILE* file, 
        memory_resource* resource) noexcept;

    // non thread safe, opening/closing FILE* is caller's responsibility
    inline output output_cstream(
        FILE* file,
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(handle_error(error_code::none)))
    {
        output out;
        error_code error{ error_code::none };
        if (!compatible()) error = error_code::incomp_api;
        else if (!memory_resource_identical()) error = error_code::mem_res_bad;
        else error = output_cstream(out, file, resource);
        handle_error(error);
        return out;
    }
}

