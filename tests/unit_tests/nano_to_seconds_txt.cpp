//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <array>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <iostream>
#include <string>
 
#include <formatter/impl/detail/nano_to_seconds_txt.hpp>

class nano_to_seconds_txt_naive {
public:
	nano_to_seconds_txt_naive() noexcept {}
	nano_to_seconds_txt_naive(uint64_t nanoseconds, int char_num) {
		assert((char_num == 0 || char_num == 2 || (char_num >= 4 && char_num <= 12)) && "Invalid char_num value!");
		minutes_ = nanoseconds / 60'000'000'000;
		uint64_t nanosecs_remain = nanoseconds - minutes_ * 60'000'000'000;

		//set string and compute minutes
		if (char_num != 0) {
			const int precision = char_num <= 3 ? 0 : char_num - 3;
			static constexpr std::array<uint64_t, 10> round{
				100500000000,
				100050000000,
				100005000000,
				100000500000,
				100000050000,
				100000005000,
				100000000500,
				100000000050,
				100000000005,
				100000000000 };
			nanosecs_remain = nanosecs_remain + round[precision];

			sec_str_.resize(12);
			std::to_chars_result result;
			result = std::to_chars(&sec_str_[0], &sec_str_[0] + 12, nanosecs_remain);
			assert(result.ec == std::errc());
			assert(result.ptr - &sec_str_[0] == 12);
			assert(sec_str_[0] == '1');

			//inserting '.'
			sec_str_[0] = sec_str_[1];
			sec_str_[1] = sec_str_[2];
			sec_str_[2] = '.';

			sec_str_.resize(char_num);

			//correcting round up to a minute
			if (sec_str_[0] == '6') {
				sec_str_[0] = '0';
				++minutes_;
			}
		}
		//round to minutes
		else {
			sec_str_.clear();
			if (nanosecs_remain >= 30'000'000'000) {
				minutes_++;
			}
		}
	}

	char const* c_str() {
		return sec_str_.c_str();
	}

	char const* data() {
		return &sec_str_[0];
	}

	int size() {
		return static_cast<int>(sec_str_.size());
	}

	uint64_t minutes() {
		return minutes_;
	}

private:
	std::string sec_str_{ "" };
	uint64_t minutes_{ 0 };
};

TEST_CASE("nano_to_seconds_txt") {
	SECTION("default_construct") {
		SECTION("production") {
			fstlog::detail::nano_to_seconds_txt msf;
			CHECK(std::chrono::duration_cast<std::chrono::minutes>(msf.minutes().time_since_epoch()).count() 
				== 0);
			CHECK(msf.second_str() == "");
		}
		SECTION("naive") {
			nano_to_seconds_txt_naive msf;
			CHECK(msf.minutes() == 0);
			CHECK(std::string_view{ msf.data(), static_cast<size_t>(msf.size()) } == "");
		}
	};

	SECTION("compare to literal") {
		// microsec, char_num, control_minute, control_str
		auto extent = GENERATE(table<long long, int, int, std::string_view>({
				std::tuple<long long, int, int, std::string_view>{0, 0, 0, ""},
				std::tuple<long long, int, int, std::string_view>{0, 2, 0, "00"},
				std::tuple<long long, int, int, std::string_view>{0, 4, 0, "00.0"},
				std::tuple<long long, int, int, std::string_view>{0, 5, 0, "00.00"},
				std::tuple<long long, int, int, std::string_view>{0, 9, 0, "00.000000"},
				std::tuple<long long, int, int, std::string_view>{0, 9, 0, "00.000000"},
				std::tuple<long long, int, int, std::string_view>{29'999'999, 0, 0, ""},
				std::tuple<long long, int, int, std::string_view>{30'000'000, 0, 1, ""},
				std::tuple<long long, int, int, std::string_view>{90'000'000, 0, 2, ""},
				std::tuple<long long, int, int, std::string_view>{90'000'000, 2, 1, "30"},
				std::tuple<long long, int, int, std::string_view>{1'060'000'001, 9, 17, "40.000001"},
				std::tuple<long long, int, int, std::string_view>{1'060'000'001, 8, 17, "40.00000"},
				std::tuple<long long, int, int, std::string_view>{1'060'000'001, 7, 17, "40.0000"},
				std::tuple<long long, int, int, std::string_view>{1'060'000'004, 9, 17, "40.000004"},
				std::tuple<long long, int, int, std::string_view>{1'060'000'004, 8, 17, "40.00000"},
				std::tuple<long long, int, int, std::string_view>{1'060'000'004, 7, 17, "40.0000"},
				std::tuple<long long, int, int, std::string_view>{1'060'000'005, 9, 17, "40.000005"},
				std::tuple<long long, int, int, std::string_view>{1'060'000'005, 8, 17, "40.00001"},
				std::tuple<long long, int, int, std::string_view>{1'060'000'005, 7, 17, "40.0000"},
				std::tuple<long long, int, int, std::string_view>{1'060'900'004, 4, 17, "40.9"},
				std::tuple<long long, int, int, std::string_view>{1'060'900'005, 2, 17, "41"},
				std::tuple<long long, int, int, std::string_view>{1'678'794'479'995'400, 2, 27'979'908, "00"},
				std::tuple<long long, int, int, std::string_view>{1'678'794'479'995'400, 6, 27'979'907, "59.995"},
				std::tuple<long long, int, int, std::string_view>{1'678'794'479'995'400, 7, 27'979'907, "59.9954"}
		}));

		long long microsecond = std::get<0>(extent);
		int char_num = std::get<1>(extent);
		int ctrl_min = std::get<2>(extent);
		std::string_view ctrl_string = std::get<3>(extent);

		SECTION("production") {
			fstlog::detail::nano_to_seconds_txt msf(
				std::chrono::system_clock::time_point{std::chrono::microseconds{microsecond}}, 
				char_num);
			CAPTURE(
				microsecond, 
				char_num, 
				std::chrono::duration_cast<std::chrono::minutes>(msf.minutes().time_since_epoch()).count(), 
				ctrl_string);
			CHECK(std::chrono::duration_cast<std::chrono::minutes>(msf.minutes().time_since_epoch()).count()
				== ctrl_min);
			
			CHECK(msf.second_str() == ctrl_string);
		};
		SECTION("naive") {
			nano_to_seconds_txt_naive msf(microsecond * 1000, char_num);
			CAPTURE(
				microsecond, 
				char_num, 
				msf.minutes(), 
				ctrl_min, ctrl_string);
			CHECK(msf.minutes() == ctrl_min);
		
			CHECK(std::string_view{ msf.data(), static_cast<size_t>(msf.size()) } == ctrl_string);
		};
	};
	
	SECTION("compare production with naive") {
		// microsec, char_num, control_minute, control_str
		long long microsecond = GENERATE(0LL, 30000000000LL, 90000000000LL, 29999999999LL, 89999999999LL);
		int char_num = GENERATE(0, 2, 4, 5, 6, 7, 8, 9);

		fstlog::detail::nano_to_seconds_txt msf(
			std::chrono::system_clock::time_point{ std::chrono::microseconds{microsecond} },
			char_num);
		nano_to_seconds_txt_naive msf_ctrl(microsecond * 1000, char_num);
		CAPTURE(
			microsecond, 
			char_num, 
			std::chrono::duration_cast<std::chrono::minutes>(msf.minutes().time_since_epoch()).count(),
			msf_ctrl.minutes());
		CHECK(std::chrono::duration_cast<std::chrono::minutes>(msf.minutes().time_since_epoch()).count()
			== msf_ctrl.minutes());
		CHECK(msf.second_str() == 
			std::string_view{ msf_ctrl.data(), static_cast<size_t>(msf_ctrl.size()) });
	};
};

TEST_CASE("nano_to_seconds_txt_random", "[.][random]") {
	
	long long m = GENERATE(take(100, random(0LL, 307445734LL)));
	long long microsecond = GENERATE(take(1000, random(0LL, 59999999999LL))) + m * 60000000;
	int char_num = GENERATE(0, 2, 6, 9);

	fstlog::detail::nano_to_seconds_txt msf(
		std::chrono::system_clock::time_point{ std::chrono::microseconds{microsecond} },
		char_num);
	nano_to_seconds_txt_naive msf_ctrl(microsecond * 1000, char_num);
	CAPTURE(microsecond, 
		char_num,
		std::chrono::duration_cast<std::chrono::minutes>(msf.minutes().time_since_epoch()).count(),
		msf_ctrl.minutes());
	CHECK(std::chrono::duration_cast<std::chrono::minutes>(msf.minutes().time_since_epoch()).count()
		== msf_ctrl.minutes());
	CHECK(msf.second_str()
		== std::string_view{ msf_ctrl.data(), static_cast<size_t>(msf_ctrl.size()) });
};
	
TEST_CASE("nano_to_seconds_txt_assert","[.]") {
	int char_num = GENERATE(-10, -1, 1, 3, 13, 100);
	DYNAMIC_SECTION("char_num_" << char_num) {
		std::cout << "seconds_to_chars " << "assert_" << char_num << "\n";
		fstlog::detail::nano_to_seconds_txt(std::chrono::system_clock::time_point{}, char_num);
	};
};

TEST_CASE("nano_to_seconds_txt_benchmark", "[.][benchmark]") {
	long long microsecond = GENERATE(59999999999LL, 6354834636352535434LL);
	DYNAMIC_SECTION("nanosec_" << microsecond) {
		int char_num = GENERATE(0, 2, 6, 9);
		DYNAMIC_SECTION("char_num_" << char_num) {
			BENCHMARK_ADVANCED("minutes_sec_frac construct")(Catch::Benchmark::Chronometer meter) {
				meter.measure([microsecond, char_num] {
					return fstlog::detail::nano_to_seconds_txt(
						std::chrono::system_clock::time_point{ std::chrono::microseconds{microsecond} },
						char_num).minutes(); });
			};
			BENCHMARK_ADVANCED("minutes_sec_frac_naive construct")(Catch::Benchmark::Chronometer meter) {
				meter.measure([microsecond, char_num] {
					return nano_to_seconds_txt_naive(microsecond * 1000, char_num).minutes(); });
			};
		};
	};
};

TEST_CASE("nano_to_seconds_txt_bruteforce", "[.][bruteforce]") {
	long long microsecond = GENERATE(range(120000000000LL, 120000001000LL));
	int char_num = GENERATE(0, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12);

	fstlog::detail::nano_to_seconds_txt msf(
		std::chrono::system_clock::time_point{ std::chrono::microseconds{microsecond} },
		char_num);
	nano_to_seconds_txt_naive msf_ctrl(microsecond * 1000, char_num);
	CAPTURE(
		microsecond, 
		char_num, 
		std::chrono::duration_cast<std::chrono::minutes>(msf.minutes().time_since_epoch()).count(),
		msf_ctrl.minutes());
	CHECK(std::chrono::duration_cast<std::chrono::minutes>(msf.minutes().time_since_epoch()).count()
		== msf_ctrl.minutes());
	CHECK(msf.second_str()	== std::string_view{ msf_ctrl.data(), static_cast<size_t>(msf_ctrl.size()) });
};

#ifdef __cpp_lib_format
TEST_CASE("nano_to_seconds_txt_pre_epoch") {
	long long microseconds = GENERATE(
		-1'264'527'351'532LL, 
		0LL,
		-1LL,
		-60'000'000LL, 
		-59'999'999LL,
		-60'000'001LL,
		-30'000'000LL,
		-30'000'001LL,
		-29'999'999LL);

	std::stringstream control;
	control << std::chrono::time_point_cast<std::chrono::duration<long long, std::ratio<1, 10000000>>>(std::chrono::system_clock::time_point{ std::chrono::microseconds{ microseconds } });

	fstlog::detail::nano_to_seconds_txt msf(
		std::chrono::system_clock::time_point{ std::chrono::microseconds{ microseconds } },
		10);
	std::stringstream result_ss;
	result_ss << std::chrono::time_point_cast<std::chrono::duration<long long, std::ratio<1, 10000000>>>(msf.minutes());
	std::string result = result_ss.str();
	CHECK(result.substr(17, 10) == "00.0000000");
	result.resize(17);
	result += msf.second_str();
	CHECK(result == control.str());
};
#endif
