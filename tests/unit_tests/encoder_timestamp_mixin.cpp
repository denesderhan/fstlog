//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <array>
#include <chrono>
#include <string_view>

#include <formatter/impl/detail/encoder_timestamp_mixin.hpp>
#include <detail/byte_span.hpp>
#include <detail/mixin/allocator_mixin.hpp>
#include <detail/mixin/error_state_mixin.hpp>
#include <formatter/impl/output_span_mixin.hpp>

using enc_type_noalign = fstlog::encoder_timestamp_mixin<false,
	fstlog::output_span_mixin<
	fstlog::error_state_mixin<
	fstlog::allocator_mixin>>>;

using enc_type_align = fstlog::encoder_timestamp_mixin<true,
	fstlog::output_span_mixin<
	fstlog::error_state_mixin<
	fstlog::allocator_mixin>>>;

static bool encode_timestamp(
	fstlog::stamp_type timestamp, 
	const char* format_str, 
	unsigned char* pos, 
	std::size_t buff_size, 
	bool local = false)
{
	if (timestamp < std::chrono::system_clock::time_point{}) {
		return false;
	}
	// forcing floor rounding to seconds
	time_t const t{ std::chrono::system_clock::to_time_t(std::chrono::floor<std::chrono::seconds>(timestamp)) };
	tm time;
#ifdef _WIN32
	if (local) {
		if (localtime_s(&time, &t) != 0) return false;
	}
	else {
		if (gmtime_s(&time, &t) != 0) return false;
	}
#else
	if (local) {
		if (localtime_r(&t, &time) == NULL) return false;
	}
	else {
		if (gmtime_r(&t, &time) == NULL) return false;
	}
#endif
	std::size_t str_size = strftime(
		reinterpret_cast<char*>(pos),
		buff_size,
		format_str,
		&time);
	if (str_size == 0) return false;
	else return true;
}

TEST_CASE("encoder_timestamp_mixin") {
	std::array<unsigned char, 128> buffer;
	
	SECTION("init_failure") {
		enc_type_noalign encoder;
		auto extent = GENERATE(table<std::string_view, fstlog::error_code>({
			std::tuple<std::string_view, fstlog::error_code>{"HEAD_%S_TAIL", fstlog::error_code::none},
			std::tuple<std::string_view, fstlog::error_code>{"wrong%?", fstlog::error_code::fmt_bad},
			std::tuple<std::string_view, fstlog::error_code>{"",fstlog::error_code::none},
			std::tuple<std::string_view, fstlog::error_code>{"x63 character long time_string 63 character long time_string 63", fstlog::error_code::none},
			std::tuple<std::string_view, fstlog::error_code>{"x64 character long time_string, 64 character long time_string 64", fstlog::error_code::str_long},
			std::tuple<std::string_view, fstlog::error_code>{"x61 chars long time_string but with seconds %S it is too long", fstlog::error_code::str_long}
			}));

		std::string_view input = std::get<0>(extent);
		CAPTURE(input);
		fstlog::buff_span_const time_format{
			reinterpret_cast<const unsigned char*>(input.data()),
			input.size() };
		auto error = std::get<1>(extent);

		auto result = encoder.init_encoder_timestamp(time_format);
		CHECK(result == error);
	};
	
	SECTION("noalign") {
		enc_type_noalign encoder;
		SECTION("zone_offset") {
			buffer.fill('!');
			// length of formatted time zone (local time)
			auto format = std::string_view{ ".0L%z" };
			fstlog::buff_span_const time_format(reinterpret_cast<const unsigned char*>(format.data()), format.size());
			auto error = encoder.init_encoder_timestamp(time_format);
			CHECK(error == fstlog::error_code::none);
			encoder.output_span_init(buffer);
			encoder.encode_timestamp(fstlog::stamp_type{});
			auto length = encoder.output_ptr() - encoder.output_begin();
			CHECK(length > 0);

			for (int ind = 0; ind < length; ind++) {
				if (buffer[ind] == '-') buffer[ind] = '+';
				else if (buffer[ind] > '0' && buffer[ind] <= '9') buffer[ind] = '0';
			}

			const auto time_offs = std::string_view(reinterpret_cast<const char*>(buffer.data()), length);
			CHECK(time_offs == "+0000");
		}

		SECTION("all_format") {
			buffer.fill('!');
			// all format spec (UTC)
			auto format = std::string_view{ ".0U%H %M %S %Y %a %d %m %y" };
			fstlog::buff_span_const time_format(reinterpret_cast<const unsigned char*>(format.data()), format.size());
			auto error = encoder.init_encoder_timestamp(time_format);
			CHECK(error == fstlog::error_code::none);
			encoder.output_span_init(buffer);
			encoder.encode_timestamp(fstlog::stamp_type{});
			auto length = encoder.output_ptr() - encoder.output_begin();
			auto time_str = std::string_view(reinterpret_cast<const char*>(buffer.data()), length);
			CHECK(time_str == "00 00 00 1970 Thu 01 01 70");
		}
	}

	SECTION("align") {
		enc_type_align encoder;
		SECTION("all_format") {
			buffer.fill('!');
			// all format spec (UTC)
			auto format = std::string_view{ "*^43.0U%H %M %S %Y %a %d %m %y" };
			fstlog::buff_span_const time_format(reinterpret_cast<const unsigned char*>(format.data()), format.size());
			auto error = encoder.init_encoder_timestamp(time_format);
			CHECK(error == fstlog::error_code::none);
			encoder.output_span_init(buffer);
			encoder.encode_timestamp(fstlog::stamp_type{});
			auto length = encoder.output_ptr() - encoder.output_begin();
			auto time_str = std::string_view(reinterpret_cast<const char*>(buffer.data()), length);
			CHECK(time_str == "********00 00 00 1970 Thu 01 01 70*********");
		}
	}

	SECTION("pre_epoch") {
		enc_type_noalign encoder;
		buffer.fill(0);
		encoder.output_span_init(buffer);
		std::string_view form_temp{ ".7U%Y-%m-%d %H:%M:%S" };
		fstlog::buff_span_const time_format{
			reinterpret_cast<const unsigned char*>(form_temp.data()),
			form_temp.size() };

		auto success = encoder.init_encoder_timestamp(time_format);
		CHECK(success == fstlog::error_code::none);
		
		long long microsecond = -1000000LL;
		CAPTURE(microsecond);
		encoder.encode_timestamp(
			std::chrono::system_clock::time_point{ std::chrono::microseconds{ microsecond } });
		CHECK(encoder.get_error().code() == fstlog::error_code::input_bad);
	}

	SECTION("buffer_size") {
		enc_type_noalign encoder;
		buffer.fill(0);
		encoder.output_span_init(buffer);
		std::string_view form_temp{ "L%z%z%z%z%z%z%z%z%z%z%z%z%z%z%z%z%z%z%z%z%z%z%z%z%z%z%z%z%z%z%zX" };
		fstlog::buff_span_const time_format{
			reinterpret_cast<const unsigned char*>(form_temp.data()),
			form_temp.size() };

		auto success = encoder.init_encoder_timestamp(time_format);
		CHECK(success == fstlog::error_code::str_long);
	}

	SECTION("compare_to_strftime") {
		std::array<unsigned char, 32> buff1{ 0 };
		std::array<unsigned char, 32> buff2{ 0 };

		const auto time_epoch = std::chrono::system_clock::time_point{};
		const auto time_now = std::chrono::system_clock::now();
		std::vector<std::chrono::system_clock::time_point> times{
			time_epoch,
			time_now,
			// second interval (rounding)
			time_epoch + std::chrono::milliseconds{500},
			time_epoch + std::chrono::milliseconds{1500},
			time_epoch + std::chrono::milliseconds{2500},
			time_epoch + std::chrono::milliseconds{15500},
			time_epoch + std::chrono::milliseconds{30500},
			time_epoch + std::chrono::milliseconds{45500},
			time_epoch + std::chrono::milliseconds{55500},
			time_now + std::chrono::milliseconds{500},
			time_now + std::chrono::milliseconds{1500},
			time_now + std::chrono::milliseconds{2500},
			time_now + std::chrono::milliseconds{15500},
			time_now + std::chrono::milliseconds{30500},
			time_now + std::chrono::milliseconds{45500},
			time_now + std::chrono::milliseconds{55500},
			// minute
			time_now + std::chrono::minutes{1},
			time_now + std::chrono::minutes{15},
			time_now + std::chrono::minutes{30},
			time_now + std::chrono::minutes{34},
			time_now + std::chrono::minutes{45},
			time_now + std::chrono::minutes{55},
			// hour
			time_now + std::chrono::hours{1},
			time_now + std::chrono::hours{5},
			time_now + std::chrono::hours{12},
			time_now + std::chrono::hours{20},
			time_now + std::chrono::hours{22},
			time_now + std::chrono::hours{24},
			// day
			time_now + std::chrono::hours{ 24 },
			time_now + std::chrono::hours{ 24 * 2 },
			time_now + std::chrono::hours{ 24 * 3 },
			time_now + std::chrono::hours{ 24 * 4 },
			time_now + std::chrono::hours{ 24 * 5 },
			time_now + std::chrono::hours{ 24 * 6 },
			// month (daylight saving if used)
			time_now + std::chrono::hours{ 24 * 30 },
			time_now + std::chrono::hours{ 24 * 30 * 2 },
			time_now + std::chrono::hours{ 24 * 30 * 3 },
			time_now + std::chrono::hours{ 24 * 30 * 4 },
			time_now + std::chrono::hours{ 24 * 30 * 5 },
			time_now + std::chrono::hours{ 24 * 30 * 6 },
			time_now + std::chrono::hours{ 24 * 30 * 7 },
			time_now + std::chrono::hours{ 24 * 30 * 8 },
			time_now + std::chrono::hours{ 24 * 30 * 9 },
			time_now + std::chrono::hours{ 24 * 30 * 10 },
			time_now + std::chrono::hours{ 24 * 30 * 11 }
		};

		SECTION("local") {
			auto strft_string = GENERATE(
				std::string_view("%Y-%m-%d %H:%M:%S %z"),
				std::string_view("%Y-%m-%d %H:%M %z"),
				std::string_view("%y.%m.%d %a %H:%M:%S %z!")
			);

			std::string encoder_string(".0L"); // 0 second precision, local
			encoder_string += strft_string;
			enc_type_noalign encoder; // do not use fill align
			fstlog::buff_span_const init_string(
				reinterpret_cast<const unsigned char*>(encoder_string.data()),
				encoder_string.size());
						
			encoder.output_span_init(buff2);
			encoder.init_encoder_timestamp(init_string);
			
			for (auto timestamp : times) {
				CAPTURE(timestamp);
				buff1.fill(0);
				buff2.fill(0);
				encoder.output_span_init(buff2);
				encoder.clear_error();

				bool success0 = encode_timestamp(timestamp, strft_string.data(), buff1.data(), buff1.size(), true);
				encoder.encode_timestamp(timestamp);
				bool success1 = !encoder.has_error();
				CHECK(success0 == success1);
				CHECK(buff1 == buff2);
			}
		}

		SECTION("UTC") {
			auto strft_string = GENERATE(
				std::string_view("%Y-%m-%d %H:%M:%S +0000"),
				std::string_view("%Y-%m-%d %H:%M +0000"),
				std::string_view("%y.%m.%d %a %H:%M:%S +0000!")
			);

			std::string encoder_string(".0U"); // 0 second precision, utc
			encoder_string += strft_string;
			enc_type_noalign encoder; // do not use fill align
			fstlog::buff_span_const init_string(
				reinterpret_cast<const unsigned char*>(encoder_string.data()),
				encoder_string.size());

			encoder.output_span_init(buff2);
			encoder.init_encoder_timestamp(init_string);

			for (auto timestamp : times) {
				CAPTURE(timestamp);
				buff1.fill(0);
				buff2.fill(0);
				encoder.output_span_init(buff2);
				encoder.clear_error();

				bool success0 = encode_timestamp(timestamp, strft_string.data(), buff1.data(), buff1.size(), false);
				encoder.encode_timestamp(timestamp);
				bool success1 = !encoder.has_error();
				CHECK(success0 == success1);
				CHECK(buff1 == buff2);
			}
		}
	}

	SECTION("second_formatting") {
		auto test_data = std::chrono::nanoseconds(59'999'999'999LL);
		auto floored_data = std::chrono::duration_cast<std::chrono::nanoseconds>(
			std::chrono::duration_cast<std::chrono::system_clock::duration>(test_data));
		std::intmax_t dat = test_data.count();
		std::intmax_t f_dat = floored_data.count();
		int zeroed_digits = 0;
		while (dat % 10 != f_dat % 10) {
			zeroed_digits++;
			dat /= 10;
			f_dat /= 10;
		}
		CHECK(zeroed_digits <= 6); // millisecond precision

		auto data = GENERATE(
			std::make_tuple(0, std::string_view(".0AAA%SBBB"), 0LL, std::string_view("AAA00BBB"), 4),
			std::make_tuple(1, ".1%S", 0, "00.0", 3),
			std::make_tuple(3, ".3%S", 0, "00.000", 5),
			std::make_tuple(9, ".9%S", 0, "00.000000000", 11),
			std::make_tuple(9, ".12AAA%SBBB", 0, "AAA00.000000000BBB", 14),
			std::make_tuple(9, ".12%S", 1000, "00.000001000", 11),
			std::make_tuple(0, ".0AAA%SBBB", 1'999'000'000, "AAA01BBB", 4),
			std::make_tuple(1, ".1%S", 1'000'000'000, "01.0", 3),
			std::make_tuple(2, ".2%S", 1'000'000'000, "01.00", 4),
			std::make_tuple(6, ".6%S", 1'000'000'000, "01.000000", 8),
			std::make_tuple(7, ".7%S", 1'000'000'000, "01.0000000", 9),
			std::make_tuple(0, ".0%S", 30'000'000'000, "30", 1),
			std::make_tuple(6, ".6%S", 2'456'129'999, "02.456129", 8),
			std::make_tuple(5, ".5%S", 40'432'015'000, "40.43201", 7),
			std::make_tuple(4, ".4%S", 40'123'470'000, "40.1234", 6),
			std::make_tuple(3, ".3%S", 49'791'654'000, "49.791", 5),
			std::make_tuple(2, ".2%S", 32'456'004'000, "32.45", 4),
			std::make_tuple(1, ".1%S", 40'678'004'000, "40.6", 3),
			std::make_tuple(7, ".7ABCDE%SFGHIJ", 123'456'789, "ABCDE00.1234567FGHIJ", 14),
			std::make_tuple(8, ".8%S", 23'364'635'251, "23.36463525", 10),
			std::make_tuple(9, ".9%S", 57'456'785'123, "57.456785123", 11),
			std::make_tuple(1, ".1%S", 40'900'004'000, "40.9", 3),
			std::make_tuple(0, ".0%S", 40'900'005'000, "40", 1),
			std::make_tuple(4, ".4%S", 59'995'450'000, "59.9954", 6),
			std::make_tuple(3, ".3%S", 59'995'450'000, "59.995", 5),
			std::make_tuple(1, ".1%S", 59'995'450'000, "59.9", 3),
			std::make_tuple(0, ".0%S", 59'995'450'000, "59", 1),
			std::make_tuple(9, ".12%S", 59'999'999'999LL, "59.999999999", 11),
			std::make_tuple(9, ".12FOO%SBAR", 56'789'123'689LL, "FOO56.789123689BAR", 14)
		);

		auto precision = std::get<0>(data);
		auto format = std::get<1>(data);
		auto nanosec = std::chrono::system_clock::time_point(
			std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::nanoseconds(std::get<2>(data))));
		std::array<unsigned char, 64> control{ 0 };
		memcpy(control.data(), std::get<3>(data).data(), std::get<3>(data).size());
		
		// zeroing out digits (chrono resolution precision loss)
		int digits_to_zero = zeroed_digits - (9 - precision);
		auto digit_pos = control.data() + std::get<4>(data);
		while (digits_to_zero-- > 0) {
			*digit_pos-- = '0';
		}

		enc_type_noalign encoder;
		std::array<unsigned char, 64> result{ 0 };
		encoder.output_span_init(result);
		encoder.init_encoder_timestamp(
			fstlog::buff_span_const(reinterpret_cast<const unsigned char*>(format.data()), format.size()));

		encoder.encode_timestamp(nanosec);
		CHECK(!encoder.has_error());
		CHECK(result == control);
	};
};

TEST_CASE("encoder_timestamp_mixin_benchmark", "[.][benchmark]") {
	std::array<unsigned char, 1024> buffer{ 0 };

	enc_type_noalign encoder_loc;
	std::string_view form_loc{ ".6L%Y-%m-%d %H:%M:%S %z" };
	fstlog::buff_span_const time_format{
			reinterpret_cast<const unsigned char*>(form_loc.data()),
			form_loc.size() };
	auto error = encoder_loc.init_encoder_timestamp(time_format);
	CHECK(error == fstlog::error_code::none);
	
	enc_type_noalign encoder_utc;
	std::string_view form_utc{ ".6U%Y-%m-%d %H:%M:%S +0000" };
	time_format = {
			reinterpret_cast<const unsigned char*>(form_utc.data()),
			form_utc.size() };
	error = encoder_utc.init_encoder_timestamp(time_format);
	CHECK(error == fstlog::error_code::none);
		
	long long seconds = 42;
	BENCHMARK_ADVANCED("local_non_cached")(Catch::Benchmark::Chronometer meter) {
		meter.measure([&buffer, &encoder_loc, &seconds] {
				encoder_loc.output_span_init(buffer);
				seconds += 60;
				encoder_loc.encode_timestamp(
					std::chrono::system_clock::time_point{ std::chrono::seconds{ seconds } });
			});
	};
	BENCHMARK_ADVANCED("local_cached")(Catch::Benchmark::Chronometer meter) {
		meter.measure([&buffer, &encoder_loc, &seconds] {
				encoder_loc.output_span_init(buffer);
				encoder_loc.encode_timestamp(
					std::chrono::system_clock::time_point{ std::chrono::seconds{ seconds } });
			});
	};
	seconds = 42;
	BENCHMARK_ADVANCED("utc_non_cached")(Catch::Benchmark::Chronometer meter) {
		meter.measure([&buffer, &encoder_utc, &seconds] {
			encoder_utc.output_span_init(buffer);
			seconds += 60;
			encoder_utc.encode_timestamp(
				std::chrono::system_clock::time_point{ std::chrono::seconds{ seconds } });
			});
	};
	BENCHMARK_ADVANCED("utc_cached")(Catch::Benchmark::Chronometer meter) {
		meter.measure([&buffer, &encoder_utc, &seconds] {
			encoder_utc.output_span_init(buffer);
			encoder_utc.encode_timestamp(
				std::chrono::system_clock::time_point{ std::chrono::seconds{ seconds } });
			});
	};
}
