//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>
#include <limits>
#include <type_traits>

#include <fstlog/detail/is_pow2.hpp>

namespace fstlog {
    // returns the number that is in the range [num, num + padd_to)
    // and is divisible by padd_to (returns 0 on num == 0)
    template <std::uintmax_t padd_to, typename T>
    constexpr T padded_size(T num) noexcept {
        static_assert(std::is_unsigned_v<T>);
        if constexpr (padd_to == 1) {
            return num;
        }
        else {
            static_assert(is_pow2(padd_to), "padd_to must be power of two!");
            static_assert(padd_to != 0 
                && padd_to <= (std::numeric_limits<T>::max)(), "padd_to must be in range 1 - maxT");
            constexpr T round = static_cast<T>(padd_to);
            // compute temp = (num / round) * round
            auto temp = num & (~(round - 1));
            // if already rounded or overflow would happen we don't padd
            if (temp == num || temp > (std::numeric_limits<T>::max)() - round) {
                return num;
            }
            else {
                return temp + round;
            }
        }
    }
}
