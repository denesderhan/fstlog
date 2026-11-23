//Copyright © 2026-present, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <chrono>
#include <optional>
#include <ratio>

namespace fstlog::detail {
    // <= 0 or > 1 day returns nullopt (switch off), other values are clamped to [50ms, 1day]
    [[nodiscard]] inline constexpr std::optional<std::chrono::milliseconds> normalized_interval(std::chrono::milliseconds interval) noexcept {
        // clamp to [50ms, 1day], allow 0 (0 switches off polling)
        constexpr auto min_interval{ std::chrono::milliseconds(50) };
        constexpr auto max_interval{ std::chrono::milliseconds(std::chrono::hours(24)) };
        constexpr auto off_interval{ std::chrono::milliseconds(0) };

        static_assert(max_interval == std::chrono::duration_cast<std::chrono::milliseconds>(
            (std::chrono::duration_cast<std::chrono::steady_clock::time_point::duration>(max_interval))),
            "Possible overflow at millisecond -> clock duration conversion!");

        // 0, negative and > 1 day maps to OFF
        if (interval <= off_interval || interval > max_interval) return std::nullopt;
        // clamp to a reasonable value
        if (interval < min_interval) return min_interval;
        return interval;
    }

    inline constexpr std::optional<std::chrono::steady_clock::time_point> next_deadline(
        std::chrono::steady_clock::time_point last_time,
        std::optional<std::chrono::milliseconds> interval) noexcept
    {
        if (!interval) return std::nullopt;
        static_assert(std::ratio_less_equal_v<std::chrono::steady_clock::time_point::period, std::milli>,
            "timer clock resolution must be at least 1ms");
        // guard against overflow
        // we use the returned value in wait_until()
        // on some systems timer_point::max() can be a sentinel that triggers a special case for wait_until()
        // so we make it a special case ourselfs (we know what will happen)  
        if (last_time >= (std::chrono::steady_clock::time_point::max)() - *interval) {
            return std::nullopt;
        }
        return last_time + *interval;
    }
} // namespace fstlog::detail
