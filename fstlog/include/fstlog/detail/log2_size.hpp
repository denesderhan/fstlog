//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <type_traits>

namespace fstlog {
    template<typename T>
    struct log2_size {
        static_assert(std::is_arithmetic_v<T>, "Unsupported type!");
        static constexpr unsigned char value = []{
            if constexpr (sizeof(T) == 1) return 0;
            else if constexpr (sizeof(T) == 2) return 1;
            else if constexpr (sizeof(T) == 4) return 2;
            else if constexpr (sizeof(T) == 8) return 3;
            else if constexpr (sizeof(T) == 16) return 4;
            else {
                static_assert(!sizeof(T), "Unsupported type!");
                return 0;
            }
        }();
    };
}
