//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <detail/unaligned_span.hpp>

namespace fstlog {
    template<typename L>
    class formatter_null_mixin : public L {
    public:
        using memory_resource_type = typename L::memory_resource_type;
        
        explicit formatter_null_mixin(memory_resource_type* resource) noexcept(
            noexcept(L(nullptr)))
            : L(resource) {}

        formatter_null_mixin(const formatter_null_mixin& other) noexcept(
            noexcept(formatter_null_mixin::get_memory_resource())
            && noexcept(formatter_null_mixin(formatter_null_mixin{nullptr}, nullptr)))
            : formatter_null_mixin(other, other.get_memory_resource()) {}
        formatter_null_mixin(const formatter_null_mixin& other, memory_resource_type* resource) noexcept(
            noexcept(L(formatter_null_mixin{nullptr}, nullptr)))
            : L(other, resource) {}

        formatter_null_mixin(formatter_null_mixin&& other) = delete;
        formatter_null_mixin& operator=(const formatter_null_mixin& rhs) = delete;
        formatter_null_mixin& operator=(formatter_null_mixin&& rhs) = delete;
        
        ~formatter_null_mixin() = default;

        static byte_span format_message(
            [[maybe_unused]] byte_span_const in,
            byte_span out) noexcept
        {
            return byte_span{ out.data_bytes(), 0 };
        }
    };
}
