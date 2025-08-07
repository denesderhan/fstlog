//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <detail/unaligned_span.hpp>

#include <array>

TEST_CASE("unaligned_span") {
	SECTION("01") {
		std::array<double, 4> data{3.14, 42.42, 0.3, std::numeric_limits<double>::infinity()};
		// +1 to the byte size to make the data unaligned
		std::array<unsigned char, sizeof(double) * data.size() + 1> buff{ 0 };
		fstlog::unaligned_span<double> test(buff.data() + 1, buff.size() - 1);
		
		// copy data
		std::size_t ind = 0;
		for (auto d : data) {
			test[ind] = d;
			ind++;
		}
		
		fstlog::unaligned_span<const double> test2(buff.data() + 1, buff.size() - 1);

		// verify
		ind = 0;
		for (auto d : data) {
			CHECK(d == test[ind]);
			CHECK(d == test2[ind]);
			ind++;
		}	
	};

}
