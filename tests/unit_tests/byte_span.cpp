//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <detail/byte_span.hpp>

#include <array>

TEST_CASE("byte_span") {
	SECTION("01") {
		std::array<unsigned char, 16> buff0{ 0, 1, 2, 3, 4, 5, 6 };
		fstlog::byte_span test0(buff0.data(), buff0.size());
		CHECK(test0.data() == buff0.data());
		CHECK(test0.size_bytes() == buff0.size());

		std::array<int, 16> buff1{ 0 };
		fstlog::byte_span test1(buff1.data(), buff1.size());
		CHECK(test1.data() == reinterpret_cast<unsigned char*>(buff1.data()));
		CHECK(test1.size_bytes() == buff1.size() * sizeof(int));

		fstlog::byte_span<int> test2(
			reinterpret_cast<unsigned char*>(buff1.data()),
			buff1.size() * sizeof(int));
		CHECK(test2.data() == reinterpret_cast<unsigned char*>(buff1.data()));
		CHECK(test2.size_bytes() == buff1.size() * sizeof(int));

		fstlog::buff_span test3(buff0);
		CHECK(test3.data() == buff0.data());
		CHECK(test3.size_bytes() == buff0.size());

		fstlog::buff_span_const test4(test3);
		CHECK(test4.data() == buff0.data());
		CHECK(test4.size_bytes() == buff0.size());

		for (int i = 0; i < 7; i++) {
			CHECK(test3[i] == i);
			test3[i] = 0;
		}

		for (int i = 0; i < 7; i++) {
			CHECK(buff0[i] == 0);
		}

		fstlog::byte_span<const int> test5(test1);
		for (int i = 0; i < 7; i++) {
			CHECK(test5[i] == 0);
		}
		//CHECK(test1[0] == 0); //This should not work!

	};

}
