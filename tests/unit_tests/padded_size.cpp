//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <limits>
 
#include <fstlog/detail/padded_size.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

template<typename T, T padd_to>
T padded_size_naive(T num) {
    static_assert(padd_to != 0);
    if (num % padd_to == 0) return num;
    std::uintmax_t padding = padd_to - (num - (num / padd_to) * padd_to);
    if (num > (std::numeric_limits<T>::max)() - padding) {
        return num;
    }
    else {
        return num + padding;
    }
}

TEST_CASE("padded_size") {
    SECTION("32") {
        auto extent = GENERATE(table<uint64_t, uint64_t>({
            std::tuple<uint64_t, uint64_t>{0, 0},
            std::tuple<uint64_t, uint64_t>{1, 32},
            std::tuple<uint64_t, uint64_t>{2, 32},
            std::tuple<uint64_t, uint64_t>{32, 32},
            std::tuple<uint64_t, uint64_t>{33, 64},
            std::tuple<uint64_t, uint64_t>{64, 64},
            std::tuple<uint64_t, uint64_t>{(uint64_t(1) << 63) - 1, (uint64_t(1) << 63)},
            std::tuple<uint64_t, uint64_t>{(uint64_t(1) << 63) + 31, (uint64_t(1) << 63) + 32},
            std::tuple<uint64_t, uint64_t>{(UINT64_MAX / 32) * 32, (UINT64_MAX / 32) * 32},
            std::tuple<uint64_t, uint64_t>{(UINT64_MAX / 32) * 32 - 1, (UINT64_MAX / 32) * 32},
            std::tuple<uint64_t, uint64_t>{(UINT64_MAX / 32) * 32 - 33, (UINT64_MAX / 32) * 32 - 32},
            std::tuple<uint64_t, uint64_t>{UINT64_MAX, UINT64_MAX},
            std::tuple<uint64_t, uint64_t>{UINT64_MAX - 1, UINT64_MAX - 1}
            }));

        uint64_t num = std::get<0>(extent);
        uint64_t expected = std::get<1>(extent);

        CAPTURE(num, expected);
        CHECK(padded_size_naive<std::uint64_t, 32>(num) == expected);
        CHECK(fstlog::padded_size<32>(num) == expected);

    };

    SECTION("padded_size_4") {
        auto extent = GENERATE(table<uint64_t, uint64_t>({
            std::tuple<uint64_t, uint64_t>{0, 0},
            std::tuple<uint64_t, uint64_t>{1, 4},
            std::tuple<uint64_t, uint64_t>{2, 4},
            std::tuple<uint64_t, uint64_t>{3, 4},
            std::tuple<uint64_t, uint64_t>{4, 4},
            std::tuple<uint64_t, uint64_t>{5, 8},
            std::tuple<uint64_t, uint64_t>{6, 8},
            std::tuple<uint64_t, uint64_t>{7, 8},
            std::tuple<uint64_t, uint64_t>{8, 8},
            std::tuple<uint64_t, uint64_t>{32, 32},
            std::tuple<uint64_t, uint64_t>{33, 36},
            std::tuple<uint64_t, uint64_t>{64, 64},
            std::tuple<uint64_t, uint64_t>{(uint64_t(1) << 63) - 1, (uint64_t(1) << 63)},
            std::tuple<uint64_t, uint64_t>{(uint64_t(1) << 63) + 3, (uint64_t(1) << 63) + 4},
            std::tuple<uint64_t, uint64_t>{(UINT64_MAX / 4) * 4, (UINT64_MAX / 4) * 4},
            std::tuple<uint64_t, uint64_t>{(UINT64_MAX / 4) * 4 - 1, (UINT64_MAX / 4) * 4},
            std::tuple<uint64_t, uint64_t>{(UINT64_MAX / 4) * 4 - 5, (UINT64_MAX / 4) * 4 - 4},
            std::tuple<uint64_t, uint64_t>{UINT64_MAX, UINT64_MAX},
            std::tuple<uint64_t, uint64_t>{UINT64_MAX - 1, UINT64_MAX - 1}
            }));

        uint64_t num = std::get<0>(extent);
        uint64_t expected = std::get<1>(extent);

        CAPTURE(num, expected);
        CHECK(padded_size_naive<std::uint64_t, 4>(num) == expected);
        CHECK(fstlog::padded_size<4>(num) == expected);
    };

    SECTION("padded_size_8") {
        auto extent = GENERATE(table<uint64_t, uint64_t>({
            std::tuple<uint64_t, uint64_t>{0, 0},
            std::tuple<uint64_t, uint64_t>{1, 8},
            std::tuple<uint64_t, uint64_t>{2, 8},
            std::tuple<uint64_t, uint64_t>{32, 32},
            std::tuple<uint64_t, uint64_t>{33, 40},
            std::tuple<uint64_t, uint64_t>{64, 64},
            std::tuple<uint64_t, uint64_t>{(uint64_t(1) << 63) - 1, (uint64_t(1) << 63)},
            std::tuple<uint64_t, uint64_t>{(uint64_t(1) << 63) + 7, (uint64_t(1) << 63) + 8},
            std::tuple<uint64_t, uint64_t>{(UINT64_MAX / 8) * 8, (UINT64_MAX / 8) * 8},
            std::tuple<uint64_t, uint64_t>{(UINT64_MAX / 8) * 8 - 1, (UINT64_MAX / 8) * 8},
            std::tuple<uint64_t, uint64_t>{(UINT64_MAX / 8) * 8 - 9, (UINT64_MAX / 8) * 8 - 8},
            std::tuple<uint64_t, uint64_t>{UINT64_MAX, UINT64_MAX},
            std::tuple<uint64_t, uint64_t>{UINT64_MAX - 1, UINT64_MAX - 1}
            }));

        uint64_t num = std::get<0>(extent);
        uint64_t expected = std::get<1>(extent);

        CAPTURE(num, expected);
        CHECK(padded_size_naive<std::uint64_t, 8>(num) == expected);
        CHECK(fstlog::padded_size<8>(num) == expected);
    };
}
