//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <fstlog/detail/log2_size.hpp>

TEST_CASE("log2_size_01") {
	SECTION("01") {
		
		CHECK(fstlog::log2_size<char>::value == 0);
		CHECK(fstlog::log2_size<uint16_t>::value == 1);
		CHECK(fstlog::log2_size<uint32_t>::value == 2);
		CHECK(fstlog::log2_size<uint64_t>::value == 3);
	};

}
