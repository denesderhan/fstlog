//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <chrono>
#include <type_traits>

#include <detail/unaligned_span.hpp>

namespace fstlog {
    template<class L>
    class sink_unsort_mixin : public L {
        typedef std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> steady_msec;
    public:
        using memory_resource_type = typename L::memory_resource_type;

        explicit sink_unsort_mixin(memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<L, memory_resource_type*>)
            : L(resource) {}

        sink_unsort_mixin(const sink_unsort_mixin& other) = delete;
        sink_unsort_mixin(sink_unsort_mixin&& other) = delete;
        sink_unsort_mixin& operator=(const sink_unsort_mixin& rhs) = delete;
        sink_unsort_mixin& operator=(sink_unsort_mixin&& rhs) = delete;

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
