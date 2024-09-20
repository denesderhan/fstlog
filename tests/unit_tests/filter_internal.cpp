//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <filter/filter_internal.hpp>

TEST_CASE("filter_internal") {

	SECTION("01") {
		fstlog::filter_internal filt;
		filt.add_channel(0);
		filt.add_level(fstlog::level::Error);
		filt.add_level(fstlog::level::Trace);
		filt.add_level(fstlog::level::Fatal);

		CHECK(filt.filter_msg(fstlog::level::Trace, 0) == true);
		CHECK(filt.filter_msg(fstlog::level::Debug, 0) == false);
		CHECK(filt.filter_msg(fstlog::level::Info, 0) == false);
		CHECK(filt.filter_msg(fstlog::level::Warn, 0) == false);
		CHECK(filt.filter_msg(fstlog::level::Error, 0) == true);
		CHECK(filt.filter_msg(fstlog::level::Fatal, 0) == true);

		filt = fstlog::filter_internal{};
		filt.add_channel(0);
		filt.add_level(fstlog::level::All, fstlog::level::None);
		CHECK(filt.filter_msg(fstlog::level::Trace, 0) == true);
		CHECK(filt.filter_msg(fstlog::level::Debug, 0) == true);
		CHECK(filt.filter_msg(fstlog::level::Info, 0) == true);
		CHECK(filt.filter_msg(fstlog::level::Warn, 0) == true);
		CHECK(filt.filter_msg(fstlog::level::Error, 0) == true);
		CHECK(filt.filter_msg(fstlog::level::Fatal, 0) == true);
		
		filt = fstlog::filter_internal{};
		filt.add_channel(0);
		filt.add_level(fstlog::level::Debug, fstlog::level::Info);
		filt.add_level(fstlog::level::Info, fstlog::level::Error);
		CHECK(filt.filter_msg(fstlog::level::Trace, 0) == false);
		CHECK(filt.filter_msg(fstlog::level::Debug, 0) == true);
		CHECK(filt.filter_msg(fstlog::level::Info, 0) == true);
		CHECK(filt.filter_msg(fstlog::level::Warn, 0) == true);
		CHECK(filt.filter_msg(fstlog::level::Error, 0) == true);
		CHECK(filt.filter_msg(fstlog::level::Fatal, 0) == false);
	}
	
	SECTION("01") {
		fstlog::filter_internal filt;

		filt.add_level(fstlog::level::All, fstlog::level::None);
		CHECK(filt.filter_msg(fstlog::level::Info, 255) == false);
		CHECK(filt.filter_msg(fstlog::level::Info, 0) == false);
		CHECK(filt.filter_msg(fstlog::level::Info, 250) == false);

		filt.add_channel(0);
		CHECK(filt.filter_msg(fstlog::level::Info, 0) == true);
		CHECK(filt.filter_msg(fstlog::level::Info, 1) == false);
		CHECK(filt.filter_msg(fstlog::level::Info, 255) == false);

		filt.add_channel(255);
		CHECK(filt.filter_msg(fstlog::level::Info, 255) == true);
		CHECK(filt.filter_msg(fstlog::level::Info, 254) == false);
		CHECK(filt.filter_msg(fstlog::level::Info, 0) == true);
		CHECK(filt.filter_msg(fstlog::level::Info, 1) == false);
		CHECK(filt.filter_msg(fstlog::level::Info, 200) == false);
	};
	
	SECTION("02") {
		fstlog::filter_internal filt;
		filt.add_level(fstlog::level::All, fstlog::level::None);
		unsigned char channel{ 0 };
		while (true) {
			if (channel % 32 == 0 || channel % 32 == 31) filt.add_channel(channel);
			if (channel == 255) break;
			channel++;
		}
		channel = 0;
		while (true) {
			if (channel % 32 == 0 || channel % 32 == 31) {
				CHECK(filt.filter_msg(fstlog::level::Info, channel) == true);
			}
			else {
				CHECK(filt.filter_msg(fstlog::level::Info, channel) == false);
			}
			if (channel == 255) break;
			channel++;
		}
	}

	SECTION("03") {
		auto extent = GENERATE(table<int, int>({
			std::tuple<int, int>{0, 30},
			std::tuple<int, int>{0, 31},
			std::tuple<int, int>{0, 32},
			std::tuple<int, int>{40, 4},
			std::tuple<int, int>{0, 63},
			std::tuple<int, int>{0, 64},
			std::tuple<int, int>{1, 65},
			std::tuple<int, int>{1, 64},
			std::tuple<int, int>{0, 255},
			std::tuple<int, int>{1, 255},
			std::tuple<int, int>{0, 254},
			std::tuple<int, int>{1, 254},
			std::tuple<int, int>{223, 255},
			std::tuple<int, int>{224, 255},
			std::tuple<int, int>{225, 255},
			std::tuple<int, int>{223, 254},
			std::tuple<int, int>{224, 254},
			std::tuple<int, int>{225, 254},
			std::tuple<int, int>{32, 31},
			std::tuple<int, int>{64, 127},
			std::tuple<int, int>{64, 128},
			std::tuple<int, int>{63, 126},
			std::tuple<int, int>{63, 127},
			std::tuple<int, int>{63, 128},
			std::tuple<int, int>{65, 126},
			std::tuple<int, int>{65, 127},
			std::tuple<int, int>{65, 128}
			}));

		fstlog::channel_type begin = static_cast<fstlog::channel_type>(std::get<0>(extent));
		fstlog::channel_type end = static_cast<fstlog::channel_type>(std::get<1>(extent));
		fstlog::filter_internal filt;
		filt.add_level(fstlog::level::All, fstlog::level::None);
		filt.add_channel(begin,end);
		auto min = begin < end ? begin : end;
		auto max = end > begin ? end : begin;
		unsigned char ch{ 0 };
		while (true) {
			CHECK(filt.filter_msg(fstlog::level::Info, ch) == (ch >= min && ch <= max));
			if (ch == 255) break;
			ch++;
		}		
	};

	SECTION("04") {
		fstlog::filter_internal filt;
		filt.add_level(fstlog::level::All, fstlog::level::None);
		filt.add_channel(20, 50);
		filt.add_channel(40, 60);
		filt.add_channel(30, 40);
		unsigned char ch{ 0 };
		while (true) {
			CHECK(filt.filter_msg(fstlog::level::Info, ch) == (ch >= 20 && ch <= 60));
			if (ch == 255) break;
			ch++;
		}
	}
}
TEST_CASE("filter_internal_rand", "[.][random]") {
	
	SECTION("one range") {
		fstlog::channel_type num1 = GENERATE(take(100, random(fstlog::channel_type{ 0 }, fstlog::channel_type{ 255 })));
		fstlog::channel_type num2 = GENERATE(take(100, random(fstlog::channel_type{ 0 }, fstlog::channel_type{ 255 })));
		CAPTURE(num1, num2);
		fstlog::filter_internal filt;
		filt.add_level(fstlog::level::All, fstlog::level::None);
		filt.add_channel(num1, num2);
		auto min = num1 < num2 ? num1 : num2;
		auto max = num1 > num2 ? num1 : num2;
		unsigned char ch{ 0 };
		while (true) {
			CHECK(filt.filter_msg(fstlog::level::Info, ch) == (ch >= min && ch <= max));
			if (ch == 255) break;
			ch++;
		}
	}

	SECTION("two range") {
		fstlog::channel_type num1 = GENERATE(take(5, random(fstlog::channel_type{ 0 }, fstlog::channel_type{ 255 })));
		fstlog::channel_type num2 = GENERATE(take(5, random(fstlog::channel_type{ 0 }, fstlog::channel_type{ 255 })));
		fstlog::channel_type num3 = GENERATE(take(5, random(fstlog::channel_type{ 0 }, fstlog::channel_type{ 255 })));
		fstlog::channel_type num4 = GENERATE(take(5, random(fstlog::channel_type{ 0 }, fstlog::channel_type{ 255 })));
		CAPTURE(num1, num2, num3, num4);
		fstlog::filter_internal filt;
		filt.add_level(fstlog::level::All, fstlog::level::None);
		filt.add_channel(num1, num2);
		filt.add_channel(num3, num4);
		auto min1 = num1 < num2 ? num1 : num2;
		auto max1 = num1 > num2 ? num1 : num2;
		auto min2 = num3 < num4 ? num3 : num4;
		auto max2 = num3 > num4 ? num3 : num4;
		unsigned char ch{ 0 };
		while (true) {
			CHECK(filt.filter_msg(fstlog::level::Info, ch) == 
				((ch >= min1 && ch <= max1) || 
					(ch >= min2 && ch <= max2))); 
			if (ch == 255) break;
			ch++;
		}
	}
}
