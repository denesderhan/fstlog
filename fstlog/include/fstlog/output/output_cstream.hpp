//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/output/output.hpp>

#include <stdio.h>

#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/memory_resource.hpp>

namespace fstlog {
    // non thread safe, opening/closing FILE* is callers responsibility
    // FILE* must be opened in binary mode
    FSTLOG_API error_code output_cstream(
        output& out,
        FILE* file, 
        memory_resource* resource) noexcept;
    // non thread safe, opening/closing FILE* is callers responsibility
    // FILE* must be opened in binary mode
    inline output output_cstream(
        FILE* file,
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(handle_error(error_code::none)))
    {
        output out;
        const auto error = output_cstream(out, file, resource);
        handle_error(error);
        return out;
    }
}

