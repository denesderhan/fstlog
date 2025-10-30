//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <type_traits>

#include <detail/unaligned_span.hpp>

namespace fstlog {
    template<class L>
    class out_null_mixin : public L {
    public:
        out_null_mixin() noexcept = default;

        out_null_mixin(const out_null_mixin& other) = delete;
        out_null_mixin(out_null_mixin&& other) = delete;
        out_null_mixin& operator=(const out_null_mixin& rhs) = delete;
        out_null_mixin& operator=(out_null_mixin&& rhs) = delete;

        ~out_null_mixin() = default;

        static void write_message([[maybe_unused]] byte_span_const msg) noexcept {}
        static void flush() noexcept {}
    };
}
