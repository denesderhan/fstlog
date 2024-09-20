//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

 
#include <fstlog/detail/padded_size.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

template<uintmax_t padd_to>
size_t padded_size_naive(uintmax_t num) {
	static_assert(padd_to != 0);
	FSTLOG_ASSERT(num <= (UINT64_MAX / padd_to) * padd_to);

	if (num == 0) return 0;
	if (num % padd_to == 0) return num;

	return ((num / padd_to) + 1) * padd_to;
}

TEST_CASE("padded_size_32") {
	SECTION("01") {
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
			std::tuple<uint64_t, uint64_t>{(UINT64_MAX / 32) * 32 - 33, (UINT64_MAX / 32) * 32 - 32}
			}));

		uint64_t num = std::get<0>(extent);
		uint64_t expected = std::get<1>(extent);

		CAPTURE(num, expected);
		CHECK(padded_size_naive<32>(num) == expected);
		CHECK(fstlog::padded_size<32>(num) == expected);

	};
}

TEST_CASE("padded_size_32_random", "[.][random]") {
	uint64_t num = GENERATE(take(100000, random(uint64_t(0), uint64_t(UINT32_MAX) *2 )));
	CAPTURE(num);
	CHECK(fstlog::padded_size<32>(num) == padded_size_naive<32>(num));
}

TEST_CASE("padded_size_4") {
	SECTION("01") {
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
			std::tuple<uint64_t, uint64_t>{(UINT64_MAX / 4) * 4 - 5, (UINT64_MAX / 4) * 4 - 4}
			}));

		uint64_t num = std::get<0>(extent);
		uint64_t expected = std::get<1>(extent);

		CAPTURE(num, expected);
		CHECK(padded_size_naive<4>(num) == expected);
		CHECK(fstlog::padded_size<4>(num) == expected);

	};
}

TEST_CASE("padded_size_4_random", "[.][random]") {
	uint64_t num = GENERATE(take(100000, random(uint64_t(0), uint64_t(UINT32_MAX) * 2)));
	CAPTURE(num);
	CHECK(fstlog::padded_size<4>(num) == padded_size_naive<4>(num));
}

TEST_CASE("padded_size_8") {
	SECTION("01") {
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
			std::tuple<uint64_t, uint64_t>{(UINT64_MAX / 8) * 8 - 9, (UINT64_MAX / 8) * 8 - 8}
			}));

		uint64_t num = std::get<0>(extent);
		uint64_t expected = std::get<1>(extent);

		CAPTURE(num, expected);
		CHECK(padded_size_naive<8>(num) == expected);
		CHECK(fstlog::padded_size<8>(num) == expected);

	};
}

TEST_CASE("padded_size_8_random", "[.][random]") {
	uint64_t num = GENERATE(take(100000, random(uint64_t(0), uint64_t(UINT32_MAX) * 2)));
	CAPTURE(num);
	CHECK(fstlog::padded_size<8>(num) == padded_size_naive<8>(num));
}
