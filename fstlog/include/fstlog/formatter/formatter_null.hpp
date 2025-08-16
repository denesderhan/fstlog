//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/formatter/formatter.hpp>

#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/fstlog_allocator.hpp>

namespace fstlog {
    FSTLOG_API error_code formatter_null(
        formatter& out,
        fstlog_allocator const& allocator) noexcept;
    inline formatter formatter_null(
        fstlog_allocator const& allocator = {}) noexcept(noexcept(handle_error(error_code::none)))
    {
        formatter out;
        const auto error = formatter_null(out, allocator);
        handle_error(error);
        return out;
    }
}
