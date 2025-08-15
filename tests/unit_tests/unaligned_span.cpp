//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <detail/unaligned_span.hpp>

#include <array>

TEST_CASE("unaligned_span") {
	SECTION("get-set-test") {
		std::array<double, 4> data{3.14, 42.42, 0.3, std::numeric_limits<double>::infinity()};
		
		// +1 to the byte size to make the data unaligned
		std::array<unsigned char, sizeof(double) * data.size() + 1> buff{ 0 };
		fstlog::unaligned_span<double> test(buff.data() + 1, buff.size() - 1);
		
		SECTION("operator[]") {
			buff.fill(0);
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

		SECTION("get-set") {
			buff.fill(0);
			// copy data
			std::size_t ind = 0;
			for (auto d : data) {
				test.set(ind, d);
				ind++;
			}

			fstlog::unaligned_span<const double> test2(buff.data() + 1, buff.size() - 1);

			// verify
			ind = 0;
			for (auto d : data) {
				CHECK(d == test.get(ind));
				CHECK(d == test2.get(ind));
				ind++;
			}
		};

		SECTION("get-set-constexpr") {
			buff.fill(0);
			// copy data
			test.template set<0>(data[0]);
			test.template set<1>(data[1]);
			test.template set<2>(data[2]);
			test.template set<3>(data[3]);
			
			fstlog::unaligned_span<const double> test2(buff.data() + 1, buff.size() - 1);

			// verify
			CHECK(data[0] == test.template get<0>());
			CHECK(data[1] == test.template get<1>());
			CHECK(data[2] == test.template get<2>());
			CHECK(data[3] == test.template get<3>());

			CHECK(data[0] == test2.template get<0>());
			CHECK(data[1] == test2.template get<1>());
			CHECK(data[2] == test2.template get<2>());
			CHECK(data[3] == test2.template get<3>());
			
		};
	};

	SECTION("const_correctness") {
		std::array<int, 4> data1{ 0 };
		auto test1 = fstlog::unaligned_span(data1);
		auto ptr1 = test1.data_bytes();
		CHECK(!std::is_const_v<std::remove_pointer_t<decltype(ptr1)>>);
		
		std::array<const int, 4> data2{ 0 };
		auto test2 = fstlog::unaligned_span(data2);
		auto ptr2 = test2.data_bytes();
		CHECK(std::is_const_v< std::remove_pointer_t<decltype(ptr2)>>);
		
		const std::array<int, 4> data3{ 0 };
		auto test3 = fstlog::unaligned_span(data3);
		auto ptr3 = test3.data_bytes();
		CHECK(std::is_const_v< std::remove_pointer_t<decltype(ptr3)>>);
		
		const std::array<const int, 4> data4{ 0 };
		auto test4 = fstlog::unaligned_span(data4);
		auto ptr4 = test4.data_bytes();
		CHECK(std::is_const_v< std::remove_pointer_t<decltype(ptr4)>>);
		

	};

	SECTION("const_span") {
		std::array<int, 4> data1{ 0 };
		const auto test1 = fstlog::unaligned_span(data1);
		test1[0] = 1;
		test1.template set<1>(0);
		test1.set(2, 2);
		
		auto d = test1[0];
		CHECK(d == 1);
		CHECK(test1.get(1) == 0);
		CHECK(test1.template get<2>() == 2);
	};

	SECTION("subspan") {
		std::array<int, 4> data{ 0 };
		auto span = fstlog::unaligned_span(data);
		CHECK(span.subspan(0, 0).empty());
		CHECK(span.subspan(0, span.size()) == span);

		auto span2 = fstlog::unaligned_span(&data[1], 2);
		CHECK(span.subspan(1, 2) == span2);

		auto span3 = fstlog::unaligned_span(&data[0], 2);
		CHECK(span.subspan(0, 2) == span3);

		auto span4 = fstlog::unaligned_span(&data[2], 2);
		CHECK(span.subspan(2, 2) == span4);
	};
		

	SECTION("drop") {
		std::array<int, 4> data{ 0 };
		auto span = fstlog::unaligned_span(data);
		auto span2 = fstlog::unaligned_span(&data[1], 2);

		auto temp = span;
		CHECK(temp.template drop_front<1>().drop_back(1) == span2);
		temp = span;
		CHECK(temp.drop_front(1).template drop_back<1>() == span2);
		temp = span;
		CHECK(temp.drop_front(3).template drop_back<1>().empty());
	};
}
