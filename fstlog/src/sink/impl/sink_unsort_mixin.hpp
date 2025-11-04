//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <chrono>

#include <detail/unaligned_span.hpp>

namespace fstlog {
    template<class L>
    class sink_unsort_mixin : public L {
        typedef std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> steady_msec;
    public:
        sink_unsort_mixin() noexcept = default;

        sink_unsort_mixin(const sink_unsort_mixin&) = delete;
        sink_unsort_mixin& operator=(const sink_unsort_mixin&) = delete;
        sink_unsort_mixin(sink_unsort_mixin&&) = delete;
        sink_unsort_mixin& operator=(sink_unsort_mixin&&) = delete;

        ~sink_unsort_mixin() = default;
        
        void sink_msg(byte_span_const message) noexcept {
            L::write_message(L::format(message));
        }

        static bool needs_immediate_flush() noexcept {
            return false;
        }

        void flush(steady_msec current_time) noexcept
        {
            this->update_flush_time(current_time);
            if (unflushed_data()) {
                L::output_flush();
                has_flushable_data_ = false;
            }
        }
        
        void set_unflushed_data() noexcept {
            has_flushable_data_ = true;
        }

    private:
        bool unflushed_data() const noexcept {
            return has_flushable_data_;
        }
        //accessed only under locked sink_store_mutex_
        bool has_flushable_data_{ false };
    };
}
