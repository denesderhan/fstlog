//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/output/output.hpp>

#include <memory>
#include <mutex>
#include <ostream>

#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/memory_resource.hpp>

namespace fstlog {
    // thread safe, ofstream must be opened in binary mode
    FSTLOG_API error_code output_stream_mt(
        output& out,
        std::shared_ptr<std::ostream> stream,
        std::shared_ptr<std::mutex> mutex,
        memory_resource* resource) noexcept;
    // thread safe, ofstream must be opened in binary mode
    inline output output_stream_mt(
        std::shared_ptr<std::ostream> stream,
        std::shared_ptr<std::mutex> mutex,
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(handle_error(error_code::none)))
    {
        output out;
        const auto error = output_stream_mt(out, stream, mutex, resource);
        handle_error(error);
        return out;
    }
}
