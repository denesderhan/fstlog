//Copyright © 2022-present, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <chrono>
#include <optional>

#ifdef FSTLOG_DEBUG
#include <fstlog/detail/fstlog_assert.hpp>
#include <detail/deadline_helper.hpp>
#endif

namespace fstlog {
    template<class L>
    class sink_flush_time_mixin : public L {
        using timer_point = std::chrono::steady_clock::time_point;
    public:
        std::optional<timer_point> next_flush_time() const noexcept {
            return next_flush_time_;
        }

        void set_next_flush_time(std::optional<timer_point> next_time) noexcept {
            next_flush_time_ = next_time;
        }
        
        std::optional<std::chrono::milliseconds> flush_interval() const noexcept {
            return flush_interval_;
        }

        void set_flush_interval( std::optional<std::chrono::milliseconds> interval) noexcept {
            FSTLOG_ASSERT(!interval || detail::normalized_interval(*interval) == *interval);
            flush_interval_ = interval;
        }
    private:
        std::optional<std::chrono::milliseconds> flush_interval_{ std::nullopt };
        std::optional<timer_point> next_flush_time_{ std::nullopt };
    };
}
