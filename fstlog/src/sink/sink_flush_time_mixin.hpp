//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <chrono>

namespace fstlog {
    template<class L>
    class sink_flush_time_mixin : public L {
        using steady_msec = std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds>;
    public:
        steady_msec next_flush_time() const noexcept {
            return next_flush_time_;
        }

        //flush() calls this but only from a core thread
        void update_flush_time(steady_msec current_time) noexcept {
            next_flush_time_ = flush_interval_ != std::chrono::milliseconds{ 0 } ?
                current_time + flush_interval_
                : (steady_msec::max)();
        }
        //only called at construction (no concurrency issue)
        void set_flush_interval( std::chrono::milliseconds interval) noexcept {
            flush_interval_ = interval;
            next_flush_time_ = interval != std::chrono::milliseconds{ 0 } ?
                std::chrono::time_point_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now())
                : (steady_msec::max)();
        }
    private:
        std::chrono::milliseconds flush_interval_{ 0 };
        steady_msec next_flush_time_{(steady_msec::max)()};
    };
}
