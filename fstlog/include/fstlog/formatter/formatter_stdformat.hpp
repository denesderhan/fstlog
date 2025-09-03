//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/formatter/formatter.hpp>

#include <string_view>

#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/memory_resource.hpp>

namespace fstlog {
    FSTLOG_API error_code formatter_stdformat(
        formatter& out, 
        memory_resource* resource) noexcept;
    FSTLOG_API error_code formatter_stdformat(
        formatter& out,
        std::string_view format_string,
        memory_resource* resource) noexcept;
    inline formatter formatter_stdformat(
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(handle_error(error_code::none)))
    {
        formatter out;
        const auto error = formatter_stdformat(out, resource);
        handle_error(error);
        return out;
    }
    inline formatter formatter_stdformat(
        std::string_view format_string,
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(handle_error(error_code::none)))
    {
        formatter out;
        const auto error = formatter_stdformat(out, format_string, resource);
        handle_error(error);
        return out;
    }
#ifdef __cpp_char8_t
    inline formatter formatter_stdformat(
        std::u8string_view format_string,
        memory_resource* resource = fstlog::get_default_resource()) noexcept(
            noexcept(handle_error(error_code::none)))
    {
        return formatter_stdformat(
            std::string_view{
                reinterpret_cast<const char*>(format_string.data()), 
                format_string.size()},
            resource);
    }
#endif
}
