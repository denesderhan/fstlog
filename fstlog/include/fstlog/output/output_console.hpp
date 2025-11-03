//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/output/output.hpp>

#include <fstlog/compatible.hpp>
#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/memory_resource.hpp>

namespace fstlog {
    FSTLOG_API error_code output_cout(
        output& out,
        memory_resource* resource) noexcept;

    FSTLOG_API error_code output_cerr(
        output& out,
        memory_resource* resource) noexcept;

    FSTLOG_API error_code output_clog(
        output& out,
        memory_resource* resource) noexcept;

    inline output output_cout(
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(handle_error(error_code::none)))
    {
        output out;
        error_code error{ error_code::none };
        if (!compatible()) error = error_code::incomp_api;
        else if (!memory_resource_identical()) error = error_code::mem_res_bad;
        else error = output_cout(out, resource);
        handle_error(error);
        return out;
    }

    inline output output_cerr(
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(handle_error(error_code::none)))
    {
        output out;
        error_code error{ error_code::none };
        if (!compatible()) error = error_code::incomp_api;
        else if (!memory_resource_identical()) error = error_code::mem_res_bad;
        else error = output_cerr(out, resource);
        handle_error(error);
        return out;
    }

    inline output output_clog(
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(handle_error(error_code::none)))
    {
        output out;
        error_code error{ error_code::none };
        if (!compatible()) error = error_code::incomp_api;
        else if (!memory_resource_identical()) error = error_code::mem_res_bad;
        else error = output_clog(out, resource);
        handle_error(error);
        return out;
    }

    inline output output_console(
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(output_cout(resource)))
    {
        return output_cout(resource);
    }
}
