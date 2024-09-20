//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/types.hpp>
#include <detail/error_code.hpp>

namespace fstlog {
    template<class L>
    class sink_null_mixin : public L {
        typedef std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> steady_msec;
    public:
        using allocator_type = typename L::allocator_type;

		sink_null_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(sink_null_mixin(allocator_type{})))
			: sink_null_mixin(allocator_type{}) {}
		explicit sink_null_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
            : L(allocator) {}

        sink_null_mixin(const sink_null_mixin& other) = delete;
        sink_null_mixin(sink_null_mixin&& other) = delete;
        sink_null_mixin& operator=(const sink_null_mixin& rhs) = delete;
        sink_null_mixin& operator=(sink_null_mixin&& rhs) = delete;

        ~sink_null_mixin() = default;

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
