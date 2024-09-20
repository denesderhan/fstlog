//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <detail/byte_span.hpp>

namespace fstlog {
    template<typename L>
    class formatter_null_mixin : public L {
    public:
        using allocator_type = typename L::allocator_type;
        
		formatter_null_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(formatter_null_mixin(allocator_type{})))
			: formatter_null_mixin(allocator_type{}) {}
        explicit formatter_null_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
			: L(allocator) {}

		formatter_null_mixin(const formatter_null_mixin& other) noexcept(
			noexcept(formatter_null_mixin::get_allocator())
			&& noexcept(formatter_null_mixin(formatter_null_mixin{}, allocator_type{})))
            : formatter_null_mixin(other, other.get_allocator()) {}
		formatter_null_mixin(const formatter_null_mixin& other, allocator_type const& allocator) noexcept(
			noexcept(L(formatter_null_mixin{}, allocator_type{})))
            : L(other, allocator) {}

        formatter_null_mixin(formatter_null_mixin&& other) = delete;
        formatter_null_mixin& operator=(const formatter_null_mixin& rhs) = delete;
        formatter_null_mixin& operator=(formatter_null_mixin&& rhs) = delete;
        
        ~formatter_null_mixin() = default;

        static buff_span format_message(
            [[maybe_unused]] buff_span_const in,
            buff_span out) noexcept
		{
            return buff_span{ out.data(), 0 };
        }
    };
}
