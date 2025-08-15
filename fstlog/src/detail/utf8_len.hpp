//Copyright © 2025, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>

namespace fstlog {
    namespace detail {
        struct utf8_len {
            std::size_t byte_len{ 0 }; // Length in bytes.
            std::size_t char_len{ 0 }; // Length in utf code points.
        };
    }
}
