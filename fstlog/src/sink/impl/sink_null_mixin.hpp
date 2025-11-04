//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <chrono>
#include <cstdint>

#include <fstlog/detail/error_code.hpp>

namespace fstlog {
    template<class L>
    class sink_null_mixin : public L {
        typedef std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> steady_msec;
    public:
        error_code sink_msg_block(
            [[maybe_unused]] const unsigned char* begin, 
            [[maybe_unused]] std::uint32_t block_size) noexcept 
        {
            return error_code::none;
        }
        
        static bool needs_immediate_flush() noexcept {
            return false;
        }
        static steady_msec next_flush_time() noexcept
        {
            return (steady_msec::max)();
        }
        static void flush([[maybe_unused]] steady_msec current_time) noexcept {}
        static void reset() noexcept {}
    };
}
