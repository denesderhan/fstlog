//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#if defined(_WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#elif defined(__linux__)
    #ifndef _GNU_SOURCE
        #define _GNU_SOURCE
    #endif
    #include <sys/types.h>
    #include <unistd.h>
#elif defined(__APPLE__) 
    #include <cstdint>
    #include <pthread.h>
#else
    #include <cstdint>
    #include <cstring>
    #include <functional>
    #include <thread>
    #include <type_traits>

    #include <detail/safe_reinterpret_cast.hpp>
#endif

namespace fstlog::this_thread {
    /**
     * Get the current thread's system ID.
     * @return Platform-specific thread identifier
     * @note Return type varies by platform
     */
    inline auto get_id() noexcept {
#if defined(_WIN32)
        return GetCurrentThreadId();
#elif defined(__linux__)
        return gettid();
#elif defined(__APPLE__)
        std::uint64_t id;
        pthread_threadid_np(NULL, &id);
        return id;
#else
        const auto data = std::this_thread::get_id();
        if constexpr (std::is_trivially_copyable_v<decltype(data)>) {
            if constexpr (sizeof(data) == sizeof(unsigned char)) {
                return *safe_reinterpret_cast<const unsigned char*>(&data);
            }
            if constexpr (sizeof(data) == sizeof(std::uint16_t)) {
                std::uint16_t id;
                std::memcpy(&id, &data, sizeof(data));
                return id;
            }
            if constexpr (sizeof(data) == sizeof(std::uint32_t)) {
                std::uint32_t id;
                std::memcpy(&id, &data, sizeof(data));
                return id;
            }
            else if constexpr (sizeof(data) == sizeof(std::uint64_t)) {
                std::uint64_t id;
                std::memcpy(&id, &data, sizeof(data));
                return id;
            }
            else {
                return std::hash<std::thread::id>{}(std::this_thread::get_id());
            }
        }
        else {
            return std::hash<std::thread::id>{}(std::this_thread::get_id());
        }
#endif
    }
}
