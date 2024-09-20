//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/detail/noexceptions.hpp>
#if defined(__cpp_lib_format) && !defined(FSTLOG_NOEXCEPTIONS)
#include <catch2/catch_all.hpp>

#include <array>
#include <cstdint>
#include <type_traits>

#include <detail/mixin/allocator_mixin.hpp>
#include <detail/mixin/error_state_mixin.hpp>
#include <formatter/impl/output_span_mixin.hpp>
#include <formatter/impl/detail/encoder_stdformat_mixin.hpp>

using enc_type = fstlog::encoder_stdformat_mixin<
					fstlog::error_state_mixin<
					fstlog::output_span_mixin<
					fstlog::allocator_mixin>>>;

TEST_CASE("encoder_stdformat_mixin") {
	SECTION("no_space_in_buffer") {
		enc_type encoder;
		std::array<unsigned char, 10> buffer{ '!' };
		buffer.fill('!');
		enc_type::format_type format{ "{}" };

		encoder.output_span_init(buffer);
		encoder.encode(std::string_view{ "This will be longer than 10 characters!" }, format);

		auto res = std::string_view(
			reinterpret_cast<char*>(encoder.output_begin()),
			encoder.output_ptr() - encoder.output_begin());
		CAPTURE(res);
		CHECK(encoder.has_error());
		CHECK(encoder.get_error().code() == fstlog::error_code::external_code_error);
		CHECK(res == "");
		CHECK(encoder.output_ptr() == encoder.output_begin());
	};

	SECTION("bool") {
		auto extent = GENERATE(table<bool, std::string_view, std::string_view>({
			std::tuple<bool, std::string_view, std::string_view>{true, "{:s}", "true"},
			std::tuple<bool, std::string_view, std::string_view>{false, "{:s}", "false"},
			//bug in msvc impl.?
			//std::tuple<bool, std::string_view, std::string_view>{false, "{:#s}", "false"},
			std::tuple<bool, std::string_view, std::string_view>{true, "{}", "true"},
			std::tuple<bool, std::string_view, std::string_view>{false,"{:}", "false"},
			//bug in msvc impl.?
			//std::tuple<bool, std::string_view, std::string_view>{false,"{:#}", "false"},
			std::tuple<bool, std::string_view, std::string_view>{true, "{:x}", "1"},
			std::tuple<bool, std::string_view, std::string_view>{false, "{:#x}", "0x0"},
			std::tuple<bool, std::string_view, std::string_view>{true, "{:X}", "1"},
			std::tuple<bool, std::string_view, std::string_view>{false, "{:#X}", "0X0"},
			std::tuple<bool, std::string_view, std::string_view>{true, "{:b}", "1"},
			std::tuple<bool, std::string_view, std::string_view>{false,"{:#b}", "0b0"},
			std::tuple<bool, std::string_view, std::string_view>{true, "{:B}", "1"},
			std::tuple<bool, std::string_view, std::string_view>{false,"{:#B}", "0B0"},
			std::tuple<bool, std::string_view, std::string_view>{true, "{:d}", "1"},
			std::tuple<bool, std::string_view, std::string_view>{false, "{:#d}", "0"},
			std::tuple<bool, std::string_view, std::string_view>{true, "{:#o}", "01"},
			std::tuple<bool, std::string_view, std::string_view>{true, "{:o}", "1"},
			std::tuple<bool, std::string_view, std::string_view>{false, "{:#o}", "0"},
			std::tuple<bool, std::string_view, std::string_view>{false, "{:o}", "0"}
			}));


		bool to_encode = std::get<0>(extent);
		enc_type::format_type format = std::get<1>(extent);
		std::string_view control = std::get<2>(extent);

		CAPTURE(to_encode, format);

		enc_type encoder;
		std::array<unsigned char, 128> buffer{ '!' };
		buffer.fill('!');
		encoder.output_span_init(buffer);
		encoder.encode(to_encode, format);
		auto res = std::string_view(
			reinterpret_cast<char*>(encoder.output_begin()),
			encoder.output_ptr() - encoder.output_begin());
		CAPTURE(res);
		CHECK(!encoder.has_error());
		CHECK(res == control);
	};

	SECTION("void*") {
		auto extent = GENERATE(table<std::uintptr_t, std::string_view, std::string_view>({
			std::tuple<std::uintptr_t, std::string_view, std::string_view>{0x12345, "{}", "0x12345"},
			std::tuple<std::uintptr_t, std::string_view, std::string_view>{0x12345, "{:}", "0x12345"},
			std::tuple<std::uintptr_t, std::string_view, std::string_view>{0x12345, "{:p}", "0x12345"}
			}));

		auto to_encode = reinterpret_cast<void*>(std::get<0>(extent));
		enc_type::format_type format = std::get<1>(extent);
		std::string_view control = std::get<2>(extent);

		CAPTURE(to_encode, format);

		enc_type encoder;
		std::array<unsigned char, 128> buffer{ '!' };
		buffer.fill('!');
		encoder.output_span_init(buffer);
		encoder.encode(to_encode, format);
		auto res = std::string_view(
			reinterpret_cast<char*>(encoder.output_begin()),
			encoder.output_ptr() - encoder.output_begin());
		CAPTURE(res);
		CHECK(!encoder.has_error());
		CHECK(res == control);
	};

	SECTION("integer") {
		auto extent = GENERATE(table<int, std::string_view, std::string_view>({
			std::tuple<int, std::string_view, std::string_view>{1234, "{:+}", "+1234"},
			std::tuple<int, std::string_view, std::string_view>{15, "{: x}", " f"},
			std::tuple<int, std::string_view, std::string_view>{15, "{:-#X}", "0XF"},
			std::tuple<int, std::string_view, std::string_view>{-15, "{:+b}", "-1111"},
			std::tuple<int, std::string_view, std::string_view>{-15, "{: #B}", "-0B1111"},
			std::tuple<int, std::string_view, std::string_view>{64, "{:-o}", "100"},
			std::tuple<int, std::string_view, std::string_view>{64, "{:+#o}", "+0100"}
			}));

		auto to_encode = std::get<0>(extent);
		std::string_view format = std::get<1>(extent);
		std::string_view control = std::get<2>(extent);

		CAPTURE(to_encode, format);

		enc_type encoder;
		std::array<unsigned char, 128> buffer{ '!' };
		buffer.fill('!');
		encoder.output_span_init(buffer);
		encoder.encode(to_encode, format);
		auto res = std::string_view(
			reinterpret_cast<char*>(encoder.output_begin()),
			encoder.output_ptr() - encoder.output_begin());
		CAPTURE(res);
		CHECK(!encoder.has_error());
		CHECK(res == control);
	};

	SECTION("floating_point") {
		using tup_typ = std::tuple<float, std::string_view, std::string_view>;
		auto extent = GENERATE(table<float, std::string_view, std::string_view>({
			tup_typ{0.12434234233422f, "{:+.4}", "+0.1243"},
			tup_typ{-1.12378f, "{:+.3e}", "-1.124e+00"},
			tup_typ{1.11f, "{:.2g}", "1.1"},
			tup_typ{1.11f, "{:.2f}", "1.11"},
			tup_typ{0.11f, "{:.2f}", "0.11"},
			tup_typ{0.11f, "{:.2g}", "0.11"},
			tup_typ{1.209f, "{:.2g}", "1.2"},
			tup_typ{0.209f, "{:.2g}", "0.21"},
			tup_typ{0.4567f, "{:.2g}", "0.46"},
			tup_typ{0.0f, "{:.3a}", "0.000p+0"},
			tup_typ{0.0f, "{:A}", "0P+0"},
			tup_typ{0.0f, "{:#.3a}", "0.000p+0"},
			tup_typ{0.0f, "{:#A}", "0.P+0"},
			tup_typ{0.0f, "{:#g}", "0.00000"}

			}));

		auto to_encode = std::get<0>(extent);
		std::string_view format = std::get<1>(extent);
		std::string_view control = std::get<2>(extent);

		CAPTURE(to_encode, format);

		enc_type encoder;
		std::array<unsigned char, 128> buffer{ '!' };
		buffer.fill('!');
		encoder.output_span_init(buffer);
		encoder.encode(to_encode, format);
		auto res = std::string_view(
			reinterpret_cast<char*>(encoder.output_begin()),
			encoder.output_ptr() - encoder.output_begin());
		CAPTURE(res);
		CHECK(!encoder.has_error());
		CHECK(res == control);
	};
}
#endif
