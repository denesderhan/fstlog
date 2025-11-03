//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/output/output.hpp>

#include <memory>
#include <mutex>
#include <ostream>

#include <fstlog/compatible.hpp>
#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/memory_resource.hpp>

namespace fstlog {
    // thread safe
    FSTLOG_API error_code output_stream_mt(
        output& out,
        std::shared_ptr<std::ostream> stream,
        std::shared_ptr<std::mutex> mutex,
        memory_resource* resource) noexcept;

    // thread safe
    inline output output_stream_mt(
        std::shared_ptr<std::ostream> stream,
        std::shared_ptr<std::mutex> mutex,
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(handle_error(error_code::none)))
    {
        output out;
        error_code error{ error_code::none };
        if (!compatible()) error = error_code::incomp_api;
        else if (!memory_resource_identical()) error = error_code::mem_res_bad;
        else error = output_stream_mt(out, stream, mutex, resource);
        handle_error(error);
        return out;
    }
}
