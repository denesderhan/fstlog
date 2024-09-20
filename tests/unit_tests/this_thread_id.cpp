//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <fstlog/logger/detail/this_thread_id.hpp>

TEST_CASE("this_thread_id") {
	SECTION("01") {
#ifdef _WIN32
		CHECK(fstlog::this_thread::get_id() == GetCurrentThreadId());
#endif

	};

}
