//Copyright © 2026, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <fstlog/detail/error_code.hpp>

namespace fstlog {
    namespace detail {
        // Returns true if the environment variable env_name exists and is not empty,
        // false otherwise (including os sys call errors)
        bool is_env_set(const char* env_name) noexcept;
#ifdef _WIN32
        // writes the value ('\0' terminated string) of the environment variable env_name to the buffer.
        // buffer must have at least buff_size bytes and buffer/env_name can't overlap
        // returns error_code::none on success
        // error_code::buff_null if the buffer is null or 0 size
        // error_code::obj_null if the environment variable does not exist (or input name was null string)
        // error_code::buff_full if the value does not fit into the buffer
        // error_code::extern_err on sys call errors.
        error_code get_env_var(const char* env_name, char* buffer, std::size_t buff_size) noexcept;
#endif
    }
}
