//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <detail/unaligned_span.hpp>

namespace fstlog {
    template<typename L>
    class formatter_null_mixin : public L {
    public:
        static byte_span format_message(
            [[maybe_unused]] byte_span_const in,
            byte_span out) noexcept
        {
            return byte_span{ out.data_bytes(), 0 };
        }
    };
}
