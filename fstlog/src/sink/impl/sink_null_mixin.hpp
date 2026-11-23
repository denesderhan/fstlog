//Copyright © 2022-present, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <chrono>
#include <cstdint>
#include <optional>

#include <fstlog/detail/error_code.hpp>

namespace fstlog {
    template<class L>
    class sink_null_mixin : public L {
                
        using timer_point = std::chrono::steady_clock::time_point;
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
        static std::optional<timer_point> next_flush_time() noexcept
        {
            return std::nullopt;
        }

        void set_next_flush_time([[maybe_unused]] std::optional<timer_point> next_time) noexcept {}

        std::optional<std::chrono::milliseconds> flush_interval() const noexcept {
            return std::nullopt;
        }

        void set_flush_interval([[maybe_unused]] std::optional<std::chrono::milliseconds> interval) noexcept {}

        static void flush() noexcept {}
        static void reset() noexcept {}
    };
} //namespace fstlog
