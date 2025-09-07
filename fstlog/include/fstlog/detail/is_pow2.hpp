//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <type_traits>

namespace fstlog {
    /**
     * @brief Checks if an unsigned integer is a power of 2
     *
     * @tparam T An unsigned integer type
     * @param num The number to check
     * @return bool true if num is a power of 2 (1, 2, 4, ...), false otherwise
     *
     * @note Returns false for 0, as 0 is not considered a power of 2
     */
    template<typename T>
    [[nodiscard]] inline constexpr bool is_pow2(T num) noexcept {
        static_assert(std::is_unsigned_v<T>, "Only for unsigned!");
        return num && ((num & (num - 1)) == 0);
    }
}
