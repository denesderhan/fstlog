//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>
#include <limits>
#include <type_traits>

#include <fstlog/detail/is_pow2.hpp>

namespace fstlog {
    namespace detail {
        /**
         * @brief Computes the closest representable power of 2 
         * to a given unsigned integer
         *
         * @tparam T An unsigned integer type
         * @param num The input number
         * @return T the closest representable power of 2 to num. In case of a tie 
         * returns the larger power of 2
         * @note This function is used for correcting bad (non power of 2) user input.
         * When the input is the expected power of 2, the time complexity is O(1).
         */
        template<typename T>
        [[nodiscard]] inline constexpr T nearest_pow2(T num) noexcept {
            static_assert(std::is_unsigned_v<T>, "T can only be unsigned!");
            // fast path
            if (is_pow2(num)) return num;
            
            // edge cases
            if (num == 0 ) return 1;
            constexpr T max_pow2 = T(1) << (std::numeric_limits<T>::digits - 1);
            if (num >= max_pow2) return max_pow2;
            
            // Start searching from 4 since 0, 1, and 2 are handled above
            T upper = 4;
            while (num > upper) upper <<= 1;
            const T lower = upper >> 1;

            // return closest
            if (num - lower < upper - num) {
                return lower;
            }
            else {
                return upper;
            }
        }
    }
}
