//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/output/output.hpp>

#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/memory_resource.hpp>

namespace fstlog {
    FSTLOG_API error_code output_null(
        output& out,
        memory_resource* resource) noexcept;
    inline output output_null(
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(handle_error(error_code::none)))
    {
        output out;
        const auto error = output_null(out, resource);
        handle_error(error);
        return out;
    }
}
