//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>
#include <chrono>
#include <time.h>
#include <charconv>
#include <iostream>
#include <sstream>
#include <string>

#include <formatter/impl/detail/time_to_str_converter.hpp>
#include <formatter/impl/detail/tz_format.hpp>

inline std::string nano_2_time_seconds_part(uint64_t nanosecs, int precision) {
	std::array<uint64_t, 10> round{ 100500000000,
									100050000000,
									100005000000,
									100000500000,
									100000050000,
									100000005000,
									100000000500,
									100000000050,
									100000000005,
									100000000000 };
	precision = std::clamp(precision, 0, 9);
	nanosecs = nanosecs % uint64_t(60000000000);
	nanosecs = nanosecs + round[precision];
	std::string out;
	out.resize(12);
	std::to_chars_result result;
	result = std::to_chars(&out[0], &out[0] + 12, nanosecs);
	//uint8_t const string_length = static_cast<uint8_t>(result.ptr - &out[0]);
	out.resize(3 + precision);
	out[0] = out[1];
	out[1] = out[2];
	out[2] = '.';
	
	if (out.size() == 3) {
		out.resize(2);
	}

	//correcting round up to a minute
	if (out[0] == '6') {
		out[0] = '0';
	}

	return out;
}

TEST_CASE("nano_2_time_seconds_part") {
	CHECK(nano_2_time_seconds_part(500000000, 0) == "01");
	CHECK(nano_2_time_seconds_part(500000001, 0) == "01");
	CHECK(nano_2_time_seconds_part(59900000000, 0) == "00");
	CHECK(nano_2_time_seconds_part(0, 2) == "00.00");
	CHECK(nano_2_time_seconds_part(61000000000, 2) == "01.00");
	CHECK(nano_2_time_seconds_part(59900000000, 2) == "59.90");
	CHECK(nano_2_time_seconds_part(6000000000, 0) == "06");
	CHECK(nano_2_time_seconds_part(10000000000, 0) == "10");
	CHECK(nano_2_time_seconds_part(11000000000, 0) == "11");
	CHECK(nano_2_time_seconds_part(60000000000, 0) == "00");
	CHECK(nano_2_time_seconds_part(599999995, 8) == "00.60000000");
	CHECK(nano_2_time_seconds_part(59999999999, 8) == "00.00000000");
};

TEST_CASE("time_to_str_converter") {
	fstlog::time_to_str_converter time_conv;
	std::array<unsigned char, 1024> buffer{ 0 };

	fstlog::tz_format tz = GENERATE(fstlog::tz_format::Local, fstlog::tz_format::UTC);

	SECTION("seconds_prec_0") {
		std::string_view form_temp{ "%S" };
		fstlog::buff_span_const time_format{
			fstlog::safe_reinterpret_cast<const unsigned char*>(form_temp.data()),
			form_temp.size() };
		
		auto success = time_conv.init_time_to_str_converter(time_format, 0, tz);
		CHECK(success == nullptr);

		auto extent = GENERATE(table<uint64_t, std::string_view>({
			std::tuple<uint64_t, std::string_view>{0, "00"},
			std::tuple<uint64_t, std::string_view>{1000000, "01"},
			std::tuple<uint64_t, std::string_view>{6000000, "06"},
			std::tuple<uint64_t, std::string_view>{10000000, "10"},
			std::tuple<uint64_t, std::string_view>{11000000, "11"},
			std::tuple<uint64_t, std::string_view>{60000000, "00"},
			std::tuple<uint64_t, std::string_view>{61000000, "01"},
			std::tuple<uint64_t, std::string_view>{999999, "01"},
			std::tuple<uint64_t, std::string_view>{999950, "01"},
			std::tuple<uint64_t, std::string_view>{1000001, "01"},
			std::tuple<uint64_t, std::string_view>{59500000, "00"},
			std::tuple<uint64_t, std::string_view>{500000, "01"}
		}));

		uint64_t microsec = std::get<0>(extent);
		std::string_view control = std::get<1>(extent);
		CAPTURE(microsec);
		buffer.fill( 0 );
		
		auto result = time_conv.timestamp_to_chars(
			std::chrono::system_clock::time_point{ std::chrono::microseconds{ microsec } }, buffer.data(), buffer.data() + buffer.size());
		CHECK(result.ec == fstlog::error_code::none);
		CHECK(std::string_view(fstlog::safe_reinterpret_cast<const char*>(buffer.data()), result.ptr - buffer.data())
			== control);
	};

	SECTION("minute_rounding") {
		std::string_view form_temp{ "%M:%S" };
		fstlog::buff_span_const time_format{
			fstlog::safe_reinterpret_cast<const unsigned char*>(form_temp.data()),
			form_temp.size() };
		
		auto success = time_conv.init_time_to_str_converter(time_format, 0, tz);
		CHECK(success == nullptr);
		buffer.fill( 0 );

		auto result = time_conv.timestamp_to_chars(
			std::chrono::system_clock::time_point{ std::chrono::microseconds{ 59900000 } },
			buffer.data(), 
			buffer.data() + buffer.size());
		CHECK(result.ec == fstlog::error_code::none);
		CHECK(std::string_view(fstlog::safe_reinterpret_cast<const char*>(buffer.data()), result.ptr - buffer.data() )
			== std::string_view{ "01:00" });
	};
}

TEST_CASE("time_to_str_converter_common") {
	fstlog::time_to_str_converter time_conv;
	std::array<unsigned char, 1024> buffer{ 0 };
	
	SECTION("misc"){
		std::string_view form_temp{ "%S" };
		fstlog::buff_span_const time_format{
			fstlog::safe_reinterpret_cast<const unsigned char*>(form_temp.data()),
			form_temp.size() };
		
		int prec = 4;
		auto success = time_conv.init_time_to_str_converter(time_format, prec);
		CHECK(success == nullptr);

		auto extent = GENERATE(table<uint64_t, std::string_view>({
			std::tuple<uint64_t, std::string_view>{1000000, "01.0000"},
			std::tuple<uint64_t, std::string_view>{500000, "00.5000"},
			std::tuple<uint64_t, std::string_view>{999999, "01.0000"},
			std::tuple<uint64_t, std::string_view>{1000001, "01.0000"},
			std::tuple<uint64_t, std::string_view>{5000001, "05.0000"},
			std::tuple<uint64_t, std::string_view>{4999999, "05.0000"}
			}));

		uint64_t microsec = std::get<0>(extent);
		std::string_view control = std::get<1>(extent);
		CAPTURE(prec, microsec);
		buffer.fill(0);

		auto result = time_conv.timestamp_to_chars(
			std::chrono::system_clock::time_point{ std::chrono::microseconds{ microsec } },
			buffer.data(),
			buffer.data() + buffer.size());
		CHECK(result.ec == fstlog::error_code::none);
		CHECK(std::string_view(fstlog::safe_reinterpret_cast<const char*>(buffer.data()), result.ptr - buffer.data() )
			== control);
	};

	SECTION("init_failure") {

		auto extent = GENERATE(table<std::string_view, bool>({
			std::tuple<std::string_view, bool>{"HEAD_%S_TAIL", true},
			std::tuple<std::string_view, bool>{"wrong%?", false},
			std::tuple<std::string_view, bool>{"",false},
			std::tuple<std::string_view, bool>{"62 character long time_string 62 character long time_string 62", true},
			std::tuple<std::string_view, bool>{"63 character long time_string 63 character long time_string 63x", false},
			std::tuple<std::string_view, bool>{"61 chars long time_string, but with seconds %S it is too long", false}
			}));

		std::string_view input = std::get<0>(extent);
		fstlog::buff_span_const time_format{
			fstlog::safe_reinterpret_cast<const unsigned char*>(input.data()),
			input.size() };
		bool result = std::get<1>(extent);

		auto success = time_conv.init_time_to_str_converter(time_format, 2);
		CHECK(result == (success == nullptr));
	};

	SECTION("timeformat") {
		
		auto extent = GENERATE(table<std::string_view, std::string_view>({
			std::tuple<std::string_view, std::string_view>{"HEAD_%S_TAIL", "HEAD_00_TAIL"},
			std::tuple<std::string_view, std::string_view>{"HEAD", "HEAD"},
			//fails with assertion
			//std::tuple<std::string_view, std::string_view>{"", "Time formatting failed"},
			std::tuple<std::string_view, std::string_view>{"HEAD_%S", "HEAD_00"},
			std::tuple<std::string_view, std::string_view>{"%S_TAIL", "00_TAIL"},
			std::tuple<std::string_view, std::string_view>{"Y%YY", "Y1970Y"},
			//fails with assertion
			//std::tuple<std::string_view, std::string_view>{"%S_HEAD_%S", "Invalid time format!"},
			std::tuple<std::string_view, std::string_view>{"%%Y%Y%%Y", "%Y1970%Y"},
			std::tuple<std::string_view, std::string_view>{"%%%Y%%Y", "%1970%Y"},
			std::tuple<std::string_view, std::string_view>{"%%%%Y%%Y", "%%Y%Y"},
			//60 chars + 2 == 62 format, 64 chars formatted > 62
			std::tuple<std::string_view, std::string_view>{"AAAAAAAAAAAAAAAAAAAAAAAAAA%YBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB%Y", "Formatted timestamp was too long!"},
			//59 chars + 2 < 62 format, 61 chars formatted + 2 > 62
			std::tuple<std::string_view, std::string_view>{"AAAAAAAAAAAAAAAAAAAAAAAAAA%YBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBA", "AAAAAAAAAAAAAAAAAAAAAAAAAA1970BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBA"},
			//60 + 2 chars format, 60 + 2 chars formatted
			std::tuple<std::string_view, std::string_view>{"AAAAAAAAAAAAAAAAAAAAAAAAAAxxBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBxx", "AAAAAAAAAAAAAAAAAAAAAAAAAAxxBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBxx"},
			//59 + 2 chars format, 61 + 2 chars formatted
			std::tuple<std::string_view, std::string_view>{"AAAAAAAAAAAAAAAAAAAAAAAAAxxBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB%Y", "AAAAAAAAAAAAAAAAAAAAAAAAAxxBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB1970"},
			//56 + 2 chars format, 60 + 2 chars formatted
			std::tuple<std::string_view, std::string_view>{"AAAAAAAAAAAAAAAAAAAAAAAAAA%YBBBBBBBBBBBBBBBBBBBBBBBBBB%Y", "AAAAAAAAAAAAAAAAAAAAAAAAAA1970BBBBBBBBBBBBBBBBBBBBBBBBBB1970"},
			//57 + 2 chars format, 63 formatted > 62
			std::tuple<std::string_view, std::string_view>{"AAAAAAAAAAAAAAAAAAAAAAAAAA%YBBBBBBBBBBBBBBBBBBBBBBBBBBB%Y", "AAAAAAAAAAAAAAAAAAAAAAAAAA1970BBBBBBBBBBBBBBBBBBBBBBBBBBB1970"}
			}));

		std::string_view input = std::get<0>(extent);
		fstlog::buff_span_const time_format{ 
			fstlog::safe_reinterpret_cast<const unsigned char*>(input.data()),
			input.size() };
		std::string_view control = std::get<1>(extent);

		auto success = time_conv.init_time_to_str_converter(time_format, 0);
		CHECK(success == nullptr);
		buffer.fill(0);
		auto result = time_conv.timestamp_to_chars(std::chrono::system_clock::time_point{}, buffer.data(), buffer.data() + buffer.size());
		CHECK(result.ec == fstlog::error_code::none);
		CHECK(std::string_view{
			fstlog::safe_reinterpret_cast<const char*>(buffer.data()),
			static_cast<std::size_t>(result.ptr - buffer.data()) }
			== control);
	};

	SECTION("local_formatting") {
		const time_t t_sec = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		auto time_p{ std::chrono::system_clock::from_time_t(t_sec) };
		
		std::string_view form_temp{ "%Y-%m-%d %H:%M:%S %z" };
		fstlog::buff_span_const time_format{
			fstlog::safe_reinterpret_cast<const unsigned char*>(form_temp.data()),
				form_temp.size() };

		auto success = time_conv.init_time_to_str_converter(time_format, 2, fstlog::tz_format::Local);
		CHECK(success == nullptr);
		buffer.fill(0);
		auto result = time_conv.timestamp_to_chars(
			time_p + std::chrono::milliseconds{ 420 }, 
			buffer.data(), buffer.data() + buffer.size());
		CHECK(result.ec == fstlog::error_code::none);
		
		tm time;
#ifdef _WIN32
		_tzset();
		localtime_s(&time, &t_sec);
#else
		tzset();
		localtime_r(&t_sec, &time); // POSIX
#endif
		const char* form{ "%Y-%m-%d %H:%M:%S.42 %z" };
		std::string control;
		control.resize(64);
		std::size_t str_size = strftime(
			control.data(),
			control.size(),
			form,
			&time);
		control.resize(str_size);

		CHECK(std::string_view(fstlog::safe_reinterpret_cast<const char*>(buffer.data()), result.ptr - buffer.data())
			== control);
	};

	SECTION("utc_formatting") {
		const auto time_p = std::chrono::system_clock::time_point(std::chrono::milliseconds{ 1716469800420LL });
		std::string_view control{ "2024-05-23 13:10:00.42 +0000" };

		std::string_view form_temp{ "%Y-%m-%d %H:%M:%S +0000" };
		fstlog::buff_span_const time_format{
			fstlog::safe_reinterpret_cast<const unsigned char*>(form_temp.data()),
				form_temp.size() };

		auto success = time_conv.init_time_to_str_converter(time_format, 2, fstlog::tz_format::UTC);
		CHECK(success == nullptr);
		buffer.fill(0);
		auto result = time_conv.timestamp_to_chars(time_p, buffer.data(), buffer.data() + buffer.size());
		CHECK(result.ec == fstlog::error_code::none);
		CHECK(std::string_view(fstlog::safe_reinterpret_cast<const char*>(buffer.data()), result.ptr - buffer.data())
			== control);
	};
}

TEST_CASE("time_to_str_converter_brute_force", "[.][brute]") {
	fstlog::time_to_str_converter time_conv;
	std::array<unsigned char, 1024> buffer{ 0 };
	
	SECTION("precision") {
		std::string_view form_temp{ "abc%Sdefghijkl" };
		fstlog::buff_span_const time_format{
			fstlog::safe_reinterpret_cast<const unsigned char*>(form_temp.data()),
				form_temp.size() };

		int prec = GENERATE(Catch::Generators::range(0, 6));
		auto success = time_conv.init_time_to_str_converter(time_format, prec);
		CHECK(success == nullptr);
		uint64_t sub = GENERATE(1, 5, 9);
		uint64_t microsec = GENERATE(Catch::Generators::range(uint64_t(1), uint64_t(10))) * uint64_t(100000) - sub;
		CAPTURE(prec, microsec);
		buffer.fill(0);

		auto result = time_conv.timestamp_to_chars(
			std::chrono::system_clock::time_point{ std::chrono::microseconds{ microsec } }, 
			buffer.data(), 
			buffer.data() + buffer.size());
		CHECK(result.ec == fstlog::error_code::none);
		CHECK(std::string_view(fstlog::safe_reinterpret_cast<const char*>(buffer.data()), result.ptr - buffer.data() )
			== std::string{ "abc" } + nano_2_time_seconds_part(microsec * 1000, prec) + "defghijkl");
	};

	SECTION("err") {
		std::string_view form_temp{ "%S" };
		fstlog::buff_span_const time_format{
			fstlog::safe_reinterpret_cast<const unsigned char*>(form_temp.data()),
				form_temp.size() };
		int prec = GENERATE(Catch::Generators::range(0, 6));
		auto success = time_conv.init_time_to_str_converter(time_format, prec);
		CHECK(success == nullptr);
		uint64_t microsec = GENERATE(399950, 99950, 500000);
		CAPTURE(prec, microsec);
		buffer.fill(0);
		
		auto result = time_conv.timestamp_to_chars(
			std::chrono::system_clock::time_point{ std::chrono::microseconds{ microsec } }, 
			buffer.data(), 
			buffer.data() + buffer.size());
		CHECK(result.ec == fstlog::error_code::none);
		CHECK(std::string_view(fstlog::safe_reinterpret_cast<const char*>(buffer.data()), result.ptr - buffer.data() )
			== nano_2_time_seconds_part(microsec * 1000, prec));
	}
}

TEST_CASE("time_to_str_converter_benchmark_seconds", "[.][benchmark]") {
	std::array<unsigned char, 1024> buffer{ 0 };
	fstlog::time_to_str_converter time_conv;
	std::string_view form_temp{ "%S" };
	fstlog::buff_span_const time_format{
		fstlog::safe_reinterpret_cast<const unsigned char*>(form_temp.data()),
		form_temp.size() };

	int prec = GENERATE(0, 4, 9);
	auto success = time_conv.init_time_to_str_converter(time_format, prec);
	CHECK(success == nullptr);
	uint64_t microsec = GENERATE(59999999, 500000, 0);

	std::cout << "PRECISION: " << prec << " MICROSEC: " << microsec <<'\n';
	BENCHMARK_ADVANCED("seconds_compute_speed")(Catch::Benchmark::Chronometer meter) {
		time_conv.timestamp_to_chars(
			std::chrono::system_clock::time_point{ std::chrono::microseconds{ microsec } },
			buffer.data(), buffer.data() + buffer.size());
		meter.measure([&buffer, &time_conv, microsec] { 
				time_conv.timestamp_to_chars(
					std::chrono::system_clock::time_point{ std::chrono::microseconds{ microsec } },
				buffer.data(), buffer.data() + buffer.size()); 
			});
	};
}

TEST_CASE("time_to_str_converter_create_timestring", "[.][benchmark]") {
	std::array<unsigned char, 1024> buffer{ 0 };
	fstlog::time_to_str_converter time_conv;
	std::string_view form_temp{ "%Y-%m-%d %H:%M:%S %z" };
	fstlog::buff_span_const time_format{
		fstlog::safe_reinterpret_cast<const unsigned char*>(form_temp.data()),
		form_temp.size() };
	auto success = time_conv.init_time_to_str_converter(time_format);
	CHECK(success == nullptr);
	long long microseconds = 0;
	BENCHMARK_ADVANCED("create_time_string")(Catch::Benchmark::Chronometer meter) {
		meter.measure([&buffer, &time_conv, &microseconds] { 
				microseconds += 61000002233; 
				time_conv.timestamp_to_chars(
					std::chrono::system_clock::time_point{ std::chrono::microseconds{ microseconds } },
					buffer.data(), buffer.data() + buffer.size());
			});
	};
}

TEST_CASE("time_to_str_converter_benchmark_sec2", "[.][benchmark]") {
	std::array<unsigned char, 1024> buffer{ 0 };
	fstlog::time_to_str_converter time_conv;
	std::string_view form_temp{ "%S" };
	fstlog::buff_span_const time_format{
		fstlog::safe_reinterpret_cast<const unsigned char*>(form_temp.data()),
		form_temp.size() };
	
	long long microsec = 10000000;
	int prec = GENERATE(range(0,9));
	auto success = time_conv.init_time_to_str_converter(time_format, prec);
	CHECK(success == nullptr);
	std::cout << "PRECISION: " << prec << " MICROSEC: " << microsec << '\n';
	BENCHMARK_ADVANCED("seconds2")(Catch::Benchmark::Chronometer meter) {
		time_conv.timestamp_to_chars(
			std::chrono::system_clock::time_point{ std::chrono::microseconds{ microsec } },
			buffer.data(), buffer.data() + buffer.size());
		meter.measure([&buffer, &time_conv, microsec] { 
				time_conv.timestamp_to_chars(
					std::chrono::system_clock::time_point{ std::chrono::microseconds{ microsec } },
					buffer.data(), buffer.data() + buffer.size()); 
			});
	};
}

TEST_CASE("time_to_str_converter_pre_epoch") {
	fstlog::time_to_str_converter time_conv;
	std::array<unsigned char, 1024> buffer{ 0 };

	std::string_view form_temp{ "%Y-%m-%d %H:%M:%S" };
	fstlog::buff_span_const time_format{
		fstlog::safe_reinterpret_cast<const unsigned char*>(form_temp.data()),
		form_temp.size() };

	auto success = time_conv.init_time_to_str_converter(time_format, 7, fstlog::tz_format::UTC);
	CHECK(success == nullptr);
	long long microsecond = GENERATE(-1LL, -1000000LL);

		CAPTURE(microsecond);
		buffer.fill(0);

		auto result = time_conv.timestamp_to_chars(
			std::chrono::system_clock::time_point{ std::chrono::microseconds{ microsecond } }, 
			buffer.data(), buffer.data() + buffer.size());
		CHECK(result.ec == fstlog::error_code::none);
		CHECK(std::string_view(fstlog::safe_reinterpret_cast<const char*>(buffer.data()), result.ptr - buffer.data())
			== "Pre epoch timestamp!");
}
