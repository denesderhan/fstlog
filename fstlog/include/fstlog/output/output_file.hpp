//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/output/output.hpp>

#include <cstdint>

#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/memory_resource.hpp>

namespace fstlog {
    FSTLOG_API error_code output_file(
        output& out,
        const char* file_path,
        memory_resource* resource) noexcept;
    inline output output_file(
        const char* file_path,
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(handle_error(error_code::none)))
    {
        output out;
        const auto error = output_file(out, file_path, resource);
        handle_error(error);
        return out;
    }

    FSTLOG_API error_code output_file(
        output& out,
        const char* file_path,
        bool truncate,
        memory_resource* resource) noexcept;
    inline output output_file(
        const char* file_path,
        bool truncate,
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(handle_error(error_code::none)))
    {
        output out;
        const auto error = output_file(out, file_path, truncate, resource);
        handle_error(error);
        return out;
    }
    FSTLOG_API error_code output_file(
        output& out,
        const char* file_path,
        bool truncate,
        std::uint32_t buffer_size,
        memory_resource* resource) noexcept;
    inline output output_file(
        const char* file_path,
        bool truncate,
        std::uint32_t buffer_size,
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(handle_error(error_code::none)))
    {
        output out;
        const auto error = output_file(out, file_path, truncate, buffer_size, resource);
        handle_error(error);
        return out;
    }
}
