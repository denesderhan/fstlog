//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <array>
#include <cstdint>
#include <type_traits>

#include <detail/byte_span.hpp>
#include <detail/mixin/allocator_mixin.hpp>
#include <detail/mixin/error_state_mixin.hpp>
#include <formatter/impl/detail/encoder_charconv_fast_mixin.hpp>
#include <formatter/impl/output_span_mixin.hpp>

using enc_type = fstlog::encoder_charconv_fast_mixin<
					fstlog::error_state_mixin<
					fstlog::output_span_mixin<
					fstlog::allocator_mixin>>>;

TEST_CASE("encoder_charconv_fast_mixin") {
	
	SECTION("no_space_in_buffer") {
		enc_type encoder;
		std::array<unsigned char, 10> buffer{'!'};
		buffer.fill('!');
		enc_type::format_type format;

		encoder.output_span_init(buffer);
		//msvc compiler bug?
		//comment out and try in release!
		CHECK(encoder.output_begin() == buffer.data());
		CHECK(encoder.output_ptr() == buffer.data());
		CHECK(encoder.output_end() == buffer.data() + 10);
		//bug end
		
		encoder.encode(std::string_view{ "This will be longer than 10 characters!" }, format);

		auto res = std::string_view(
			reinterpret_cast<char*>(encoder.output_begin()),
			encoder.output_ptr() - encoder.output_begin());
		CAPTURE(res);
		CHECK(encoder.has_error());
		CHECK(encoder.get_error().code() == fstlog::error_code::no_space_in_buffer);
		CHECK(res == "");
		CHECK(encoder.output_ptr() == encoder.output_begin());
	};

	SECTION("bool") {
		auto extent = GENERATE(table<bool, char, std::string_view>({
			std::tuple<bool, char, std::string_view>{true, 's', "true"},
			std::tuple<bool, char, std::string_view>{false, 's', "false"},
			std::tuple<bool, char, std::string_view>{true, '\0', "true"},
			std::tuple<bool, char, std::string_view>{false, '\0', "false"},
			std::tuple<bool, char, std::string_view>{true, 'x', "0x1"},
			std::tuple<bool, char, std::string_view>{false, 'x', "0x0"},
			std::tuple<bool, char, std::string_view>{true, 'X', "0x1"},
			std::tuple<bool, char, std::string_view>{false, 'X', "0x0"},
			std::tuple<bool, char, std::string_view>{true, 'b', "0b1"},
			std::tuple<bool, char, std::string_view>{false, 'b', "0b0"},
			std::tuple<bool, char, std::string_view>{true, 'B', "0b1"},
			std::tuple<bool, char, std::string_view>{false, 'B', "0b0"},
			std::tuple<bool, char, std::string_view>{true, 'd', "1"},
			std::tuple<bool, char, std::string_view>{false, 'd', "0"},
			std::tuple<bool, char, std::string_view>{true, 'o', "01"},
			std::tuple<bool, char, std::string_view>{true, 'o', "01"},
			std::tuple<bool, char, std::string_view>{false, 'o', "0"},
			std::tuple<bool, char, std::string_view>{false, 'o', "0"}
			}));


		bool to_encode = std::get<0>(extent);
		auto format = enc_type::format_type{};
		format.type = static_cast<unsigned char>(std::get<1>(extent));
		std::string_view control = std::get<2>(extent);

		CAPTURE(to_encode, char(format.type));

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
		auto extent = GENERATE(table<std::uintptr_t, char, std::string_view>({
			std::tuple<std::uintptr_t, char, std::string_view>{0x12345, 'p', "0x12345"},
			std::tuple<std::uintptr_t, char, std::string_view>{0x12345, '\0', "0x12345"}
			}));


		auto to_encode = reinterpret_cast<void*>(std::get<0>(extent));
		auto format = enc_type::format_type{};
		format.type = static_cast<unsigned char>(std::get<1>(extent));
		std::string_view control = std::get<2>(extent);

		CAPTURE(to_encode, char(format.type));

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
		auto extent = GENERATE(table<int, char, char, bool, std::string_view>({
			std::tuple<int, char,  char, bool, std::string_view>{1234, '\0', '+', false, "1234"},
			std::tuple<int, char,  char, bool, std::string_view>{15, 'x', ' ', false, "0xf"},
			std::tuple<int, char,  char, bool, std::string_view>{15, 'X', '-', true, "0xf"},
			std::tuple<int, char,  char, bool, std::string_view>{-15, 'b', '+', false, "-0b1111"},
			std::tuple<int, char,  char, bool, std::string_view>{-15, 'B', ' ', true, "-0b1111"},
			std::tuple<int, char,  char, bool, std::string_view>{64, 'o', '-', false, "0100"},
			std::tuple<int, char,  char, bool, std::string_view>{64, 'o', '+', true, "0100"}
			}));

		auto to_encode = std::get<0>(extent);
		auto format = enc_type::format_type{};
		format.type = static_cast<unsigned char>(std::get<1>(extent));
		std::string_view control = std::get<4>(extent);

		CAPTURE(to_encode, char(format.type));

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
		//                                type           sign           alternate   precision
		using tup_typ = std::tuple<float, char, char, bool, int, std::string_view>;
		auto extent = GENERATE(table<float, char, char, bool, int, std::string_view>({
			tup_typ{0.12434234233422f, '\0', '+', false, 4, "0.1243"},
			tup_typ{-1.12378f, 'e', '+', false, 3, "-1.124e+00"},
			tup_typ{1.11f, 'g', '-', false, 2, "1.1"},
			tup_typ{1.11f, 'f', '-', false, 2, "1.11"},
			tup_typ{0.11f, 'f', '-', false, 2, "0.11"},
			tup_typ{0.11f, 'g', '-', false, 2, "0.11"},
			tup_typ{1.209f, 'g', '-', false, 2, "1.2"},
			tup_typ{0.209f, 'g', '-', false, 2, "0.21"},
			tup_typ{0.4567f, 'g', '-', false, 2, "0.46"},
			tup_typ{0.0f, 'a', '-', false, 3, "0.000p+0"},
			tup_typ{0.0f, 'A', '-', false, enc_type::format_type{}.precision, "0p+0"},
			tup_typ{0.0f, 'a', '-', true, 3, "0.000p+0"}
			//alternate not implemented
			//tup_typ{0.0f, 'A', '-', true, enc_type::format_type{}.precision, "0.P+0"},
			//tup_typ{0.0f, 'g', '-', true, enc_type::format_type{}.precision, "0.00000"}

			}));

		auto to_encode = std::get<0>(extent);
		auto format = enc_type::format_type{};
		format.type = static_cast<unsigned char>(std::get<1>(extent));
		format.precision = static_cast<std::uint16_t>(std::get<4>(extent));
		std::string_view control = std::get<5>(extent);

		CAPTURE(to_encode, char(format.type), format.precision);

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
