//Copyright © 2026, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <limits>

#include <detail/get_env.hpp>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <stdlib.h>
#endif

namespace fstlog {
    namespace detail {

#ifdef _WIN32
        bool is_env_set(const char* env_name) noexcept {
            // only a 0 length string will fit in 1 char
            char first_char{ '\0' };
            auto result = get_env_var(env_name, &first_char, 1);

            if (result == error_code::buff_full) return true; // at least 1 character + terminating '\0'
            return false; // error_code::none on an empty string ('\0') or other errors
        }
        
        error_code get_env_var(const char* env_name, char* buffer, std::size_t buff_size) noexcept {
            if (buffer == nullptr || buff_size == 0) return error_code::buff_null; // output buffer null or 0 size
            buffer[0] = '\0';
            if (env_name == nullptr || env_name[0] == '\0') return error_code::obj_null; // environment variable name null
            
            constexpr auto dword_max = (std::numeric_limits<DWORD>::max)();
            DWORD usable_size = buff_size > dword_max ? dword_max : static_cast<DWORD>(buff_size);
            SetLastError(ERROR_SUCCESS);
            auto length = GetEnvironmentVariableA(env_name, buffer, usable_size);
            if (length == 0) { // error or empty
                buffer[0] = '\0';
                const DWORD error = GetLastError();
                if (error == ERROR_SUCCESS) return error_code::none; // empty value
                if (error == ERROR_ENVVAR_NOT_FOUND) return error_code::obj_null;
                return error_code::extern_err;
            }
            
            // on success length does not include the terminating '\0'
            // but length + 1 must fit in the buffer
            if (length < usable_size) return error_code::none;
                
            buffer[0] = '\0';
            return error_code::buff_full; // output buffer too small
        }
#else
        bool is_env_set(const char* env_name) noexcept {
            if (env_name == nullptr || env_name[0] == '\0') return false;
            auto val = ::getenv(env_name);
            if (val == nullptr || val[0] == '\0') return false;
            return true;
        }
#endif
    }
}
