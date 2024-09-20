//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

 
#include <fstlog/logger/detail/log_element_counter_overhead.hpp>

TEST_CASE("log_element_counter_overhead") {
	SECTION("01") {
		
		int test_int{ 0 };
		constexpr auto oh_int = fstlog::log_element_counter_overhead(test_int);
		CHECK(oh_int == 0);
	};
}
