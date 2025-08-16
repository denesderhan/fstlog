//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/output/output.hpp>

#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/fstlog_allocator.hpp>

namespace fstlog {
    FSTLOG_API error_code output_cout(
        output& out,
        fstlog_allocator const& allocator = {}) noexcept;
    FSTLOG_API error_code output_cerr(
        output& out,
        fstlog_allocator const& allocator = {}) noexcept;
    FSTLOG_API error_code output_clog(
        output& out,
        fstlog_allocator const& allocator = {}) noexcept;
    inline output output_cout(
        fstlog_allocator const& allocator = {}) noexcept(noexcept(handle_error(error_code::none)))
    {
        output out;
        const auto error = output_cout(out, allocator);
        handle_error(error);
        return out;
    }
    inline output output_cerr(
        fstlog_allocator const& allocator = {}) noexcept(noexcept(handle_error(error_code::none)))
    {
        output out;
        const auto error = output_cerr(out, allocator);
        handle_error(error);
        return out;
    }
    inline output output_clog(
        fstlog_allocator const& allocator = {}) noexcept(noexcept(handle_error(error_code::none)))
    {
        output out;
        const auto error = output_clog(out, allocator);
        handle_error(error);
        return out;
    }
    inline output output_console(
        fstlog_allocator const& allocator = {}) noexcept(noexcept(output_cout()))
    {
        return output_cout(allocator);
    }
}
