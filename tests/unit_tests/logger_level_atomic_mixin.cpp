//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <fstlog/logger/detail/logger_base_mixin.hpp>
#include <fstlog/logger/detail/logger_level_atomic_mixin.hpp>

using logger_test =
	fstlog::logger_level_atomic_mixin<
	fstlog::logger_base_mixin >;


TEST_CASE("logger_level_atomic_mixin") {
	SECTION("construct") {
		logger_test l1;
		CHECK(l1.level() == fstlog::level::All);
		l1.set_level(fstlog::level::Fatal);
		CHECK(l1.level() == fstlog::level::Fatal);
		l1.set_level(fstlog::level::Info);
		CHECK(l1.level() == fstlog::level::Info);
		
		auto l2{ l1 };
		CHECK(l1.level() == fstlog::level::Info);
		CHECK(l2.level() == fstlog::level::Info);

		auto l3{ std::move(l1) };
		CHECK(l1.level() == fstlog::level::Info);
		CHECK(l3.level() == fstlog::level::Info);
	};

	SECTION("assignment") {
		logger_test l1;
		l1.set_level(fstlog::level::Info);
		CHECK(l1.level() == fstlog::level::Info);
		
		logger_test l2;
		CHECK(l2.level() == fstlog::level::All);
		l2 = l1;
		CHECK(l1.level() == fstlog::level::Info);
		CHECK(l2.level() == fstlog::level::Info);

		logger_test l3;
		l3 = std::move(l1);
		CHECK(l1.level() == fstlog::level::Info);
		CHECK(l3.level() == fstlog::level::Info);
	};
}
