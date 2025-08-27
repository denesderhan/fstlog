//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <cstdint>

#include <fstlog/detail/log2_size.hpp>

TEST_CASE("log2_size") { 
    CHECK(fstlog::log2_size<std::uint8_t>::value == 0);
    CHECK(fstlog::log2_size<std::uint16_t>::value == 1);
    CHECK(fstlog::log2_size<std::uint32_t>::value == 2);
    CHECK(fstlog::log2_size<std::uint64_t>::value == 3);
}
