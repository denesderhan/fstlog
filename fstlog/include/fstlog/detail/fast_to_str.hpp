//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <limits>
#include <type_traits>

#include <fstlog/detail/small_string.hpp>

namespace fstlog {
    template<typename T>
    constexpr fstlog::small_string<32> to_hex(T num) noexcept {
        static_assert(
            std::is_integral_v<T> 
            && std::is_unsigned_v<T>
            && std::numeric_limits<T>::digits <= 64,
            "type not supported!");
        //64bit max value can have maximum 16 hexadecimal digits + "0x" 18 chars
        char buffer[18]{};
        const auto buff_end = &buffer[0] + sizeof(buffer);
        auto c_ptr{ buff_end };
        constexpr char digits[]{ 
            '0','1','2','3','4','5','6','7','8','9',
            'a','b','c','d','e','f' };
        do {
            *--c_ptr = digits[num & 0xf];
            num >>= 4;
        } while (num != 0);
        *--c_ptr = 'x';
        *--c_ptr = '0';
        return { c_ptr, static_cast<std::size_t>(buff_end - c_ptr) };
    }

    template<typename T>
    constexpr fstlog::small_string<32> to_dec(T num) noexcept {
        static_assert(
            std::is_integral_v<T>
            && std::is_unsigned_v<T>
            && std::numeric_limits<T>::digits <= 64,
            "type not supported!");
        //64bit max value can have maximum 20 decimal digits
        char buffer[20]{};
        const auto buff_end = &buffer[0] + sizeof(buffer);
        auto c_ptr{ buff_end };
        constexpr char digits2[]{
            "0001020304050607080910111213141516171819"
            "2021222324252627282930313233343536373839"
            "4041424344454647484950515253545556575859"
            "6061626364656667686970717273747576777879"
            "8081828384858687888990919293949596979899" };

        do {
            T div_num = num / 100;
            int ind = static_cast<int>(num - (div_num * 100)) * 2;
            *(--c_ptr) = digits2[ind + 1];
            *(--c_ptr) = digits2[ind];
            num = div_num;
        } while (num != 0);
        if (*c_ptr == '0') {
            c_ptr++;
        }
        return { c_ptr, static_cast<std::size_t>(buff_end - c_ptr) };
    }
}
