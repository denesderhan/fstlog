//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <cstddef> // __cpp_lib_format is defined here in windows
#if not defined(__cpp_lib_format) && (defined(__cplusplus) && __cplusplus >= 202000L)
#include <format> // __cpp_lib_format is defined here in gcc if __cplusplus >= 202000L
#endif
#include <fstlog/detail/noexceptions.hpp>
#if defined(__cpp_lib_format) && !defined(FSTLOG_NOEXCEPTIONS)
#include <catch2/catch_all.hpp>

#include <vector>
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

using utf8_char_std = std::remove_const_t<std::remove_pointer_t<std::decay_t<decltype(u8"")>>>;
using utf8_char_lib = std::conditional<std::is_signed_v<utf8_char_std>, std::make_unsigned<utf8_char_std>::type, utf8_char_std>::type;

TEST_CASE("encoder_stdformat_mixin") {
	enc_type encoder;
	std::vector<unsigned char> buffer(128, '!');
	encoder.clear_error();
	encoder.output_span_init(fstlog::buff_span(buffer.data(), buffer.size()));
	encoder.encode(std::string_view{ "string" }, enc_type::format_type{ "{:®<10}" });
	bool stdformat_utf_fill_char = !encoder.has_error();
	
	SECTION("no_space_in_buffer") {
		buffer.resize(10, '!');
		fstlog::buff_span out_buff(buffer.data(), buffer.size());

		enc_type::format_type format{ "{}" };

		encoder.clear_error();
		encoder.output_span_init(out_buff);
		encoder.encode(std::string_view{ "This will be longer than 10 characters!" }, format);

		auto res = std::string_view(
			reinterpret_cast<char*>(encoder.output_begin()),
			encoder.output_ptr() - encoder.output_begin());
		CAPTURE(res);
		CHECK(encoder.has_error());
		CHECK(encoder.get_error().code() == fstlog::error_code::extern_err);
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

		buffer.resize(128, '!');
		fstlog::buff_span out_buff(buffer.data(), buffer.size());
		encoder.clear_error();
		encoder.output_span_init(out_buff);
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

		buffer.resize(128, '!');
		fstlog::buff_span out_buff(buffer.data(), buffer.size());
		encoder.clear_error();
		encoder.output_span_init(out_buff);
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
			std::tuple<int, std::string_view, std::string_view>{(std::numeric_limits<std::int32_t>::min)(), "{:+}", "-2147483648"},
			std::tuple<int, std::string_view, std::string_view>{0, "{:+}", "+0"},
			std::tuple<int, std::string_view, std::string_view>{0, "{:-}", "0"},
			std::tuple<int, std::string_view, std::string_view>{1234, "{:+}", "+1234"},
			std::tuple<int, std::string_view, std::string_view>{-15, "{: x}", "-f"},
			std::tuple<int, std::string_view, std::string_view>{15, "{: #x}", " 0xf"},
			std::tuple<int, std::string_view, std::string_view>{15, "{:-#X}", "0XF"},
			std::tuple<int, std::string_view, std::string_view>{15, "{:-X}", "F"},
			std::tuple<int, std::string_view, std::string_view>{-15, "{:+#b}", "-0b1111"},
			std::tuple<int, std::string_view, std::string_view>{-15, "{:+b}", "-1111"},
			std::tuple<int, std::string_view, std::string_view>{-15, "{: #B}", "-0B1111"},
			std::tuple<int, std::string_view, std::string_view>{15, "{: B}", " 1111"},
			std::tuple<int, std::string_view, std::string_view>{64, "{:-o}", "100"},
			std::tuple<int, std::string_view, std::string_view>{64, "{:+#o}", "+0100"}
			}));

		auto to_encode = std::get<0>(extent);
		std::string_view format = std::get<1>(extent);
		std::string_view control = std::get<2>(extent);

		CAPTURE(to_encode, format);

		buffer.resize(128, '!');
		fstlog::buff_span out_buff(buffer.data(), buffer.size());
		encoder.clear_error();
		encoder.output_span_init(out_buff);
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
			tup_typ{1.12378f, "{: .3e}", " 1.124e+00"},
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

		buffer.resize(128, '!');
		fstlog::buff_span out_buff(buffer.data(), buffer.size());
		encoder.clear_error();
		encoder.output_span_init(out_buff);
		encoder.encode(to_encode, format);
		auto res = std::string_view(
			reinterpret_cast<char*>(encoder.output_begin()),
			encoder.output_ptr() - encoder.output_begin());
		CAPTURE(res);
		CHECK(!encoder.has_error());
		CHECK(res == control);
	};

	SECTION("char") {
		auto format = "{:}";
		buffer.resize(256, '!');
		fstlog::buff_span out_buff(buffer.data(), buffer.size());
		
		SECTION("default_format") {
			encoder.clear_error();
			encoder.output_span_init(out_buff);

			encoder.encode(char{ 'A' }, format);
			encoder.encode(char{ 0 }, format);
			encoder.encode(char16_t{ 'B' }, format);
			encoder.encode(char16_t{ 0xFFFD }, format);
			encoder.encode(char32_t{ 'C' }, format);
			encoder.encode(char32_t{ 0xD800 }, format);
			auto res = std::basic_string_view(
				reinterpret_cast<const utf8_char_lib*>(encoder.output_begin()),
				encoder.output_ptr() - encoder.output_begin());
			CHECK(!encoder.has_error());
			CHECK(res == u8"A\\u0000B\\uFFFDC\\uFFFD");
		}

		if (stdformat_utf_fill_char) {

			SECTION("fill_align_utf") {
				encoder.clear_error();
				encoder.output_span_init(out_buff);
				format = "{:₰^10}";

				encoder.encode(char{ 'A' }, format);
				encoder.encode(char{ 0 }, format);
				encoder.encode(char16_t{ 'B' }, format);
				encoder.encode(char16_t{ 0xFFFD }, format);
				encoder.encode(char32_t{ 'C' }, format);
				encoder.encode(char32_t{ 0xD800 }, format);
				auto res = std::basic_string_view(
					reinterpret_cast<const utf8_char_lib*>(encoder.output_begin()),
					encoder.output_ptr() - encoder.output_begin());
				CHECK(!encoder.has_error());
				CHECK(res == u8"₰₰₰₰A₰₰₰₰₰₰₰\\u0000₰₰₰₰₰₰B₰₰₰₰₰₰₰\\uFFFD₰₰₰₰₰₰C₰₰₰₰₰₰₰\\uFFFD₰₰");
			}

			SECTION("fill_align_precision_utf") {
				encoder.clear_error();
				encoder.output_span_init(out_buff);
				format = "{:¤>4.5}";

				encoder.encode(char{ 'A' }, format);
				encoder.encode(char{ 0 }, format);
				encoder.encode(char16_t{ u'§' }, format);
				encoder.encode(char16_t{ 0xFFFD }, format);
				encoder.encode(char32_t{ U'®' }, format);
				encoder.encode(char32_t{ 0xD800 }, format);
				auto res = std::basic_string_view(
					reinterpret_cast<const utf8_char_lib*>(encoder.output_begin()),
					encoder.output_ptr() - encoder.output_begin());
				CHECK(!encoder.has_error());
				CHECK(res == u8"¤¤¤A\\u000¤¤¤§\\uFFF¤¤¤®\\uFFF");
			}
		}
		
		SECTION("fill_align") {
			encoder.clear_error();
			encoder.output_span_init(out_buff);
			format = "{:x^10}";

			encoder.encode(char{ 'A' }, format);
			encoder.encode(char{ 0 }, format);
			encoder.encode(char16_t{ 'B' }, format);
			encoder.encode(char16_t{ 0xFFFD }, format);
			encoder.encode(char32_t{ 'C' }, format);
			encoder.encode(char32_t{ 0xD800 }, format);
			auto res = std::basic_string_view(
				reinterpret_cast<const utf8_char_lib*>(encoder.output_begin()),
				encoder.output_ptr() - encoder.output_begin());
			CHECK(!encoder.has_error());
			CHECK(res == u8"xxxxAxxxxxxx\\u0000xxxxxxBxxxxxxx\\uFFFDxxxxxxCxxxxxxx\\uFFFDxx");
		}

		SECTION("fill_align_precision") {
			encoder.clear_error();
			encoder.output_span_init(out_buff);
			format = "{:x>4.5}";

			encoder.encode(char{ 'A' }, format);
			encoder.encode(char{ 0 }, format);
			encoder.encode(char16_t{ u'q' }, format);
			encoder.encode(char16_t{ 0xFFFD }, format);
			encoder.encode(char32_t{ U'w' }, format);
			encoder.encode(char32_t{ 0xD800 }, format);
			auto res = std::basic_string_view(
				reinterpret_cast<const utf8_char_lib*>(encoder.output_begin()),
				encoder.output_ptr() - encoder.output_begin());
			CHECK(!encoder.has_error());
			CHECK(res == u8"xxxA\\u000xxxq\\uFFFxxxw\\uFFF");
		}
		
	}

	SECTION("string") {
		buffer.resize(128, '!');
		fstlog::buff_span out_buff(buffer.data(), buffer.size());
		auto format = "{:}";
		

		SECTION("default_format") {
			encoder.clear_error();
			encoder.output_span_init(out_buff);

			encoder.encode(std::basic_string_view("ASCII string\n"), format);
			encoder.encode(std::basic_string_view(u8"UTF-8 string§©"), format);
			encoder.encode(std::basic_string_view(u"UTF-16 string§©"), format);
			encoder.encode(std::basic_string_view(U"UTF-32 string§©"), format);

			auto res = std::basic_string_view(
				reinterpret_cast<const utf8_char_lib*>(encoder.output_begin()),
				encoder.output_ptr() - encoder.output_begin());
			CHECK(!encoder.has_error());
			CHECK(res == u8"ASCII string\\u000AUTF-8 string§©UTF-16 string§©UTF-32 string§©");
		}

		if (stdformat_utf_fill_char) {
			SECTION("fill_align_utf") {
				encoder.clear_error();
				encoder.output_span_init(out_buff);
				format = "{:©<20}";

				encoder.encode(std::basic_string_view("ASCII string\n"), format);
				encoder.encode(std::basic_string_view(u8"UTF-8 string§©"), format);
				encoder.encode(std::basic_string_view(u"UTF-16 string§©"), format);
				encoder.encode(std::basic_string_view(U"UTF-32 string§©"), format);

				auto res = std::basic_string_view(
					reinterpret_cast<const utf8_char_lib*>(encoder.output_begin()),
					encoder.output_ptr() - encoder.output_begin());
				CHECK(!encoder.has_error());
				CHECK(res == u8"ASCII string\\u000A©©UTF-8 string§©©©©©©©UTF-16 string§©©©©©©UTF-32 string§©©©©©©");
			}
		
			SECTION("fill_align_precision_utf") {
				encoder.clear_error();
				encoder.output_span_init(out_buff);
				format = "{:.^10.6}";
			
				encoder.encode(std::basic_string_view("ASCII string\n"), format);
				encoder.encode(std::basic_string_view(u8"§©UTF-8 string§©"), format);
				encoder.encode(std::basic_string_view(u"§©UTF-16 string§©"), format);
				encoder.encode(std::basic_string_view(U"§©UTF-32 string§©"), format);

				auto res = std::basic_string_view(
					reinterpret_cast<const utf8_char_lib*>(encoder.output_begin()),
					encoder.output_ptr() - encoder.output_begin());
				CHECK(!encoder.has_error());
				CHECK(res == u8"..ASCII ....§©UTF-....§©UTF-....§©UTF-..");
			}
		}
		
		SECTION("fill_align") {
			encoder.clear_error();
			encoder.output_span_init(out_buff);
			format = "{:x<20}";

			encoder.encode(std::basic_string_view("ASCII string\n"), format);
			encoder.encode(std::basic_string_view(u8"UTF-8 stringqw"), format);
			encoder.encode(std::basic_string_view(u"UTF-16 stringqw"), format);
			encoder.encode(std::basic_string_view(U"UTF-32 stringqw"), format);

			auto res = std::basic_string_view(
				reinterpret_cast<const utf8_char_lib*>(encoder.output_begin()),
				encoder.output_ptr() - encoder.output_begin());
			CHECK(!encoder.has_error());
			CHECK(res == u8"ASCII string\\u000AxxUTF-8 stringqwxxxxxxUTF-16 stringqwxxxxxUTF-32 stringqwxxxxx");
		}

		SECTION("fill_align_precision") {
			encoder.clear_error();
			encoder.output_span_init(out_buff);
			format = "{:.^10.6}";

			encoder.encode(std::basic_string_view("ASCII string\n"), format);
			encoder.encode(std::basic_string_view(u8"qwUTF-8 stringqw"), format);
			encoder.encode(std::basic_string_view(u"qwUTF-16 stringqw"), format);
			encoder.encode(std::basic_string_view(U"qwUTF-32 stringqw"), format);

			auto res = std::basic_string_view(
				reinterpret_cast<const utf8_char_lib*>(encoder.output_begin()),
				encoder.output_ptr() - encoder.output_begin());
			CHECK(!encoder.has_error());
			CHECK(res == u8"..ASCII ....qwUTF-....qwUTF-....qwUTF-..");
		}
	}

	SECTION("nanosec_epoch") {
		buffer.resize(128, '!');
		fstlog::buff_span out_buff(buffer.data(), buffer.size());
		std::string_view time_fmt = ".2U%Y-%m-%d %H:%M:%S";
		fstlog::buff_span_const time_format(reinterpret_cast<const unsigned char*>(time_fmt.data()), time_fmt.size());
		encoder.init_time_to_str_converter(time_format);
		auto format = "{:}";
		

		SECTION("default_format") {
			encoder.clear_error();
			encoder.output_span_init(out_buff);

			encoder.encode(std::chrono::system_clock::time_point{}, format);
			encoder.encode(std::chrono::system_clock::time_point{ std::chrono::system_clock::duration{std::chrono::seconds{60}} }, format);
			encoder.encode(std::chrono::system_clock::time_point{ std::chrono::system_clock::duration{std::chrono::milliseconds{1751021567230}} }, format);

			auto res = std::basic_string_view(
				reinterpret_cast<const utf8_char_lib*>(encoder.output_begin()),
				encoder.output_ptr() - encoder.output_begin());
			CHECK(!encoder.has_error());
			CHECK(res == u8"1970-01-01 00:00:00.001970-01-01 00:01:00.002025-06-27 10:52:47.23");

			encoder.encode(std::chrono::system_clock::time_point{ std::chrono::system_clock::duration{std::chrono::seconds{-60}} }, format);
			CHECK(encoder.get_error().code() == fstlog::error_code::input_bad);
		}

		SECTION("fill_align_precision") {
			encoder.clear_error();
			encoder.output_span_init(out_buff);
			format = "{:.<26.19}";
			
			encoder.encode(std::chrono::system_clock::time_point{ std::chrono::system_clock::duration{std::chrono::milliseconds{1751021567230}} }, format);

			auto res = std::basic_string_view(
				reinterpret_cast<const utf8_char_lib*>(encoder.output_begin()),
				encoder.output_ptr() - encoder.output_begin());
			CHECK(!encoder.has_error());
			CHECK(res == u8"2025-06-27 10:52:47.......");
		}
	}

	SECTION("reencode_tail_string") {
		buffer.resize(128, '!');
		fstlog::buff_span out_buff(buffer.data(), buffer.size());
		auto format = "{:}";
		encoder.clear_error();
		encoder.output_span_init(out_buff);

		encoder.encode(std::string_view("String left untouched."), format);
		auto str_begin = encoder.output_ptr();
		encoder.encode(std::string_view("String to \"shift fill\"."), format);
		auto res = std::basic_string_view(
			reinterpret_cast<const utf8_char_lib*>(encoder.output_begin()),
			encoder.output_ptr() - encoder.output_begin());
		CHECK(res == u8"String left untouched.String to \"shift fill\".");
		
		format = "{:x>10.6}";
		encoder.reencode_tail_string(str_begin, format);
		res = std::basic_string_view(
			reinterpret_cast<const utf8_char_lib*>(encoder.output_begin()),
			encoder.output_ptr() - encoder.output_begin());
		CHECK(res == u8"String left untouched.xxxxString");
		
		if (stdformat_utf_fill_char) {
			format = "{:¤>10.6}";
			encoder.reencode_tail_string(str_begin, format);
			res = std::basic_string_view(
				reinterpret_cast<const utf8_char_lib*>(encoder.output_begin()),
				encoder.output_ptr() - encoder.output_begin());
			CHECK(res == u8"String left untouched.¤¤¤¤xxxxSt");
		}
	}
}
#endif
