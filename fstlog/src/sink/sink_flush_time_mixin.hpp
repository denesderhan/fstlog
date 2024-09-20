//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <chrono>

namespace fstlog {
    template<class L>
    class sink_flush_time_mixin : public L {
        typedef std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> steady_msec;
    public:
        using allocator_type = typename L::allocator_type;

		sink_flush_time_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(sink_flush_time_mixin(allocator_type{})))
			: sink_flush_time_mixin(allocator_type{}) {}
        explicit sink_flush_time_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
            : L(allocator) {}

        sink_flush_time_mixin(const sink_flush_time_mixin& other) = delete;
        sink_flush_time_mixin(sink_flush_time_mixin&& other) = delete;
        sink_flush_time_mixin& operator=(const sink_flush_time_mixin& rhs) = delete;
        sink_flush_time_mixin& operator=(sink_flush_time_mixin&& rhs) = delete;

        ~sink_flush_time_mixin() = default;

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
