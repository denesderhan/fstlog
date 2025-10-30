//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <type_traits>

#include <detail/unaligned_span.hpp>

namespace fstlog {
    template<typename L>
    class formatter_null_mixin : public L {
    public:
        formatter_null_mixin() noexcept = default;

        formatter_null_mixin(const formatter_null_mixin& other) noexcept(
            noexcept(other.get_memory_resource())
            && std::is_nothrow_constructible_v<
                formatter_null_mixin,
                const formatter_null_mixin&,
                memory_resource*>)
            : formatter_null_mixin(other, other.get_memory_resource()) {}
        formatter_null_mixin(const formatter_null_mixin& other, memory_resource* resource) noexcept(
            std::is_nothrow_constructible_v<
                L,
                const L&,
                memory_resource*>)
            : L(static_cast<const L&>(other), resource) {}

        formatter_null_mixin(formatter_null_mixin&&) = delete;
        formatter_null_mixin& operator=(const formatter_null_mixin&) = delete;
        formatter_null_mixin& operator=(formatter_null_mixin&&) = delete;
        
        ~formatter_null_mixin() = default;

        static byte_span format_message(
            [[maybe_unused]] byte_span_const in,
            byte_span out) noexcept
        {
            return byte_span{ out.data_bytes(), 0 };
        }
    };
}
