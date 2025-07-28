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

TEST_CASE("encoder_timestamp_mixin") {
	std::array<unsigned char, 128> buffer;
	
	SECTION("init_failure") {
		enc_type_noalign encoder;
		auto extent = GENERATE(table<std::string_view, fstlog::error_code>({
			std::tuple<std::string_view, fstlog::error_code>{"HEAD_%S_TAIL", fstlog::error_code::none},
			std::tuple<std::string_view, fstlog::error_code>{"wrong%?", fstlog::error_code::fmt_bad},
			std::tuple<std::string_view, fstlog::error_code>{"",fstlog::error_code::none},
			std::tuple<std::string_view, fstlog::error_code>{"x62character long time_string 62 character long time_string 62", fstlog::error_code::none},
			std::tuple<std::string_view, fstlog::error_code>{"x63 character long time_string 63 character long time_string 63", fstlog::error_code::str_long},
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

			auto time_offs = std::string_view(reinterpret_cast<const char*>(buffer.data()), length);
			bool good = time_offs == "+0000" || time_offs == "+00:00";
			CHECK(good);
		}

		SECTION("all_format") {
			buffer.fill('!');
			// all format spec (UTC)
			auto format = std::string_view{ ".0U%H %I %M %S %U %W %Y %d %j %m %w %y" };
			fstlog::buff_span_const time_format(reinterpret_cast<const unsigned char*>(format.data()), format.size());
			auto error = encoder.init_encoder_timestamp(time_format);
			CHECK(error == fstlog::error_code::none);
			encoder.output_span_init(buffer);
			encoder.encode_timestamp(fstlog::stamp_type{});
			auto length = encoder.output_ptr() - encoder.output_begin();
			auto time_str = std::string_view(reinterpret_cast<const char*>(buffer.data()), length);
			CHECK(time_str == "00 12 00 00 00 00 1970 01 001 01 4 70");
		}
	}

	SECTION("align") {
		enc_type_align encoder;
		SECTION("all_format") {
			buffer.fill('!');
			// all format spec (UTC)
			auto format = std::string_view{ "*^43.0U%H %I %M %S %U %W %Y %d %j %m %w %y" };
			fstlog::buff_span_const time_format(reinterpret_cast<const unsigned char*>(format.data()), format.size());
			auto error = encoder.init_encoder_timestamp(time_format);
			CHECK(error == fstlog::error_code::none);
			encoder.output_span_init(buffer);
			encoder.encode_timestamp(fstlog::stamp_type{});
			auto length = encoder.output_ptr() - encoder.output_begin();
			auto time_str = std::string_view(reinterpret_cast<const char*>(buffer.data()), length);
			CHECK(time_str == "***00 12 00 00 00 00 1970 01 001 01 4 70***");
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
};

TEST_CASE("encoder_timestamp_mixin_benchmark", "[.][benchmark]") {
	std::array<unsigned char, 1024> buffer{ 0 };
	enc_type_noalign encoder;
	std::string_view form_temp{ "%Y-%m-%d %H:%M:%S %z" };
	fstlog::buff_span_const time_format{
		reinterpret_cast<const unsigned char*>(form_temp.data()),
		form_temp.size() };
	auto success = encoder.init_encoder_timestamp(time_format);
	CHECK(success == fstlog::error_code::none);
	long long microseconds = 0;
	BENCHMARK_ADVANCED("non_cached")(Catch::Benchmark::Chronometer meter) {
		meter.measure([&buffer, &encoder, &microseconds] {
				encoder.output_span_init(buffer);
				microseconds += 61000002233; 
				encoder.encode_timestamp(
					std::chrono::system_clock::time_point{ std::chrono::microseconds{ microseconds } });
			});
	};
	BENCHMARK_ADVANCED("cached")(Catch::Benchmark::Chronometer meter) {
		meter.measure([&buffer, &encoder, &microseconds] {
				encoder.output_span_init(buffer);
				encoder.encode_timestamp(
					std::chrono::system_clock::time_point{ std::chrono::microseconds{ microseconds } });
			});
	};
}
