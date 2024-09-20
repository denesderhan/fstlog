//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <detail/byte_span.hpp>

namespace fstlog {
    template<class L>
    class out_null_mixin : public L {
    public:
        using allocator_type = typename L::allocator_type;

        out_null_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(out_null_mixin(allocator_type{})))
			: out_null_mixin(allocator_type{}) {}
		explicit out_null_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
            : L(allocator) {}

        out_null_mixin(const out_null_mixin& other) = delete;
        out_null_mixin(out_null_mixin&& other) = delete;
        out_null_mixin& operator=(const out_null_mixin& rhs) = delete;
        out_null_mixin& operator=(out_null_mixin&& rhs) = delete;

        ~out_null_mixin() = default;

        static void write_message([[maybe_unused]] buff_span_const msg) noexcept {}
        static void flush() noexcept {}
    };
}
