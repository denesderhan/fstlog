//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <fstlog/logger/detail/logger_base_mixin.hpp>
#include <fstlog/logger/detail/logger_dropcount_mixin.hpp>

using logger_test =
	fstlog::logger_dropcount_mixin<
	fstlog::logger_base_mixin >;


TEST_CASE("logger_dropcount_mixin") {
	SECTION("construct") {
		logger_test l1;
		for (int i = 0; i < 100; i++) {
			l1.count_dropped();
		}
		CHECK(l1.dropped() == 100);
		auto l2{ l1 };
		CHECK(l1.dropped() == 100);
		CHECK(l2.dropped() == 0);

		auto l3{ std::move(l1) };
		CHECK(l1.dropped() == 0);
		CHECK(l3.dropped() == 100);
	};

	SECTION("assignment") {
		logger_test l1;
		CHECK(l1.dropped() == 0);
		for (int i = 0; i < 100; i++) {
			l1.count_dropped();
		}
		CHECK(l1.dropped() == 100);
		logger_test l2;
		CHECK(l2.dropped() == 0);
		l2 = l1;
		CHECK(l1.dropped() == 100);
		CHECK(l2.dropped() == 0);

		logger_test l3;
		CHECK(l3.dropped() == 0);
		l3 = std::move(l1);
		CHECK(l1.dropped() == 0);
		CHECK(l3.dropped() == 100);
	};
}
