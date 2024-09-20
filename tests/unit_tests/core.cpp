//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <fstlog/core.hpp>

TEST_CASE("core_id") {
	SECTION("01") {
		fstlog::core core_1{ "core_1" };
		fstlog::core core_2{ "core_2" };
		fstlog::core core_3{ nullptr };
		fstlog::core core_4{ nullptr };
		CHECK(core_1.id() != core_2.id());
		CHECK(core_1.id() != core_3.id());
		CHECK(core_3.id() == core_4.id());

		core_3 = fstlog::core{ "core_3" };
		CHECK(core_3.id() != core_4.id());
		CHECK(core_3.id() != core_2.id());
		
		core_4 = core_3;
		CHECK(core_3.pimpl() == core_4.pimpl());
	};
}
