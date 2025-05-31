//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma execution_character_set("utf-8")
#include <cstring>
#include <limits>
#include <string_view>
#include <type_traits>

#include <catch2/catch_all.hpp>

#include <detail/utf_conv.hpp>

using utf8_char_std = std::remove_const_t<std::remove_pointer_t<std::decay_t<decltype(u8"")>>>;
using utf8_char_lib = std::conditional<std::is_signed_v<utf8_char_std>, std::make_unsigned<utf8_char_std>::type, utf8_char_std>::type;

TEST_CASE("utf_conv") {
	constexpr std::basic_string_view<utf8_char_std> u8_text{
		u8"123 UTF-8 encoded sample ∮ E⋅da = Q,  n → ∞, ∑ f(i)"
		u8"= ∏ g(i)ði ıntəˈnæʃənəl fəˈnɛtık əsoʊsiˈeıʃn ‘single’ and"
		u8"“double” quotes• †, ‡, ‰, •, 3–4, —, −5 / +5, ™, … χαῖρε, ὦ χαῖρε, "
		u8"᾿Ελευθεριά!ტექსტების დამუშავებასა და მრავალენოვან ᚻᛖ ᚳᚹᚫᚦ ᚦᚫᛏ ᚻᛖ ᛒᚢᛞᛖ ᚩᚾ\u00a0"
		u8"ᚦᚫᛗ ᛚᚪᚾᛞᛖ⡕⠇⠙ ⡍⠜⠇⠑⠹ ⠺⠁⠎ ⠁⠎ ⠙⠑⠁⠙ ⠁⠎ ⠁ ⠙⠕⠕⠗⠤⠝⠁⠊⠇⠲ コンニチハ"
		u8"𒄀𠀀𐀀😀🼀𯼀𰀀𿼀􀀀􏼀󰀀󿼀🧪∛�𝄢𝄞𝅘𝅥𝅮🀺🁉🖒🖂🕱" };
	constexpr std::basic_string_view <char16_t> u16_text{
		u"123 UTF-8 encoded sample ∮ E⋅da = Q,  n → ∞, ∑ f(i)"
		u"= ∏ g(i)ði ıntəˈnæʃənəl fəˈnɛtık əsoʊsiˈeıʃn ‘single’ and"
		u"“double” quotes• †, ‡, ‰, •, 3–4, —, −5 / +5, ™, … χαῖρε, ὦ χαῖρε, "
		u"᾿Ελευθεριά!ტექსტების დამუშავებასა და მრავალენოვან ᚻᛖ ᚳᚹᚫᚦ ᚦᚫᛏ ᚻᛖ ᛒᚢᛞᛖ ᚩᚾ\u00a0"
		u"ᚦᚫᛗ ᛚᚪᚾᛞᛖ⡕⠇⠙ ⡍⠜⠇⠑⠹ ⠺⠁⠎ ⠁⠎ ⠙⠑⠁⠙ ⠁⠎ ⠁ ⠙⠕⠕⠗⠤⠝⠁⠊⠇⠲ コンニチハ"
		u"𒄀𠀀𐀀😀🼀𯼀𰀀𿼀􀀀􏼀󰀀󿼀🧪∛�𝄢𝄞𝅘𝅥𝅮🀺🁉🖒🖂🕱" };
	constexpr std::basic_string_view <char32_t> u32_text{
		U"123 UTF-8 encoded sample ∮ E⋅da = Q,  n → ∞, ∑ f(i)"
		U"= ∏ g(i)ði ıntəˈnæʃənəl fəˈnɛtık əsoʊsiˈeıʃn ‘single’ and"
		U"“double” quotes• †, ‡, ‰, •, 3–4, —, −5 / +5, ™, … χαῖρε, ὦ χαῖρε, "
		U"᾿Ελευθεριά!ტექსტების დამუშავებასა და მრავალენოვან ᚻᛖ ᚳᚹᚫᚦ ᚦᚫᛏ ᚻᛖ ᛒᚢᛞᛖ ᚩᚾ\u00a0"
		U"ᚦᚫᛗ ᛚᚪᚾᛞᛖ⡕⠇⠙ ⡍⠜⠇⠑⠹ ⠺⠁⠎ ⠁⠎ ⠙⠑⠁⠙ ⠁⠎ ⠁ ⠙⠕⠕⠗⠤⠝⠁⠊⠇⠲ コンニチハ"
		U"𒄀𠀀𐀀😀🼀𯼀𰀀𿼀􀀀􏼀󰀀󿼀🧪∛�𝄢𝄞𝅘𝅥𝅮🀺🁉🖒🖂🕱" };
	constexpr auto text_char_num{ u32_text.size() };
	static_assert(text_char_num == 323, "!");
	
	SECTION("utf32_to_utf8") {
		std::array<utf8_char_lib, 1024> out_buff{ 0 };
		std::size_t char_num = GENERATE(0, 150, 323, (std::numeric_limits<std::size_t>::max)());
		CAPTURE(char_num);
		{
			const auto wanted_charnum{ char_num };
			const unsigned char* in_begin{ reinterpret_cast<const unsigned char*>(u32_text.data()) };
			const auto result = fstlog::detail::utf32_to_utf8<char32_t, utf8_char_lib>(
				in_begin, in_begin + (u32_text.size() * sizeof(char32_t)),
				out_buff.data(), out_buff.data() + out_buff.size(),
				char_num);
			CHECK(result.ec == fstlog::error_code::none);
			if (wanted_charnum == (std::numeric_limits<std::size_t>::max)())
				CHECK(char_num == text_char_num);
			else CHECK(char_num == wanted_charnum);

			const auto wanted_trim{ char_num };
			auto trimmed_num{ wanted_trim };
			const auto trimmed_end = fstlog::utf8_str_trim(
				u8_text.data(),
				u8_text.data() + u8_text.size(),
				trimmed_num);
			CHECK(wanted_trim == trimmed_num);

			const std::basic_string_view<utf8_char_std> trimmed_control{
				u8_text.data(),
				static_cast<std::size_t>(trimmed_end - u8_text.data())
			};

			CHECK(trimmed_control ==
				std::basic_string_view<utf8_char_std>{
					reinterpret_cast<utf8_char_std*>(out_buff.data()),
					static_cast<std::size_t>(result.ptr - out_buff.data())});
		}
	};

	SECTION("utf16_to_utf8") {
		std::array<utf8_char_lib, 1024> out_buff{ 0 };
		std::size_t char_num = GENERATE(0, 150, 323, (std::numeric_limits<std::size_t>::max)());
		CAPTURE(char_num);
		{
			const auto wanted_charnum{ char_num };
			const unsigned char* in_begin{ reinterpret_cast<const unsigned char*>(u16_text.data()) };
			const auto result = fstlog::detail::utf16_to_utf8<char16_t, utf8_char_lib>(
				in_begin, in_begin + (u16_text.size() * sizeof(char16_t)),
				out_buff.data(), out_buff.data() + out_buff.size(),
				char_num);
			CHECK(result.ec == fstlog::error_code::none);
			if(wanted_charnum == (std::numeric_limits<std::size_t>::max)())
				CHECK(char_num == text_char_num);
			else CHECK(char_num == wanted_charnum);
			
			const auto wanted_trim{ char_num };
			auto trimmed_num{ wanted_trim };
			const auto trimmed_end = fstlog::utf8_str_trim(
					u8_text.data(),
					u8_text.data() + u8_text.size(),
					trimmed_num);
			CHECK(wanted_trim == trimmed_num);

			const std::basic_string_view<utf8_char_std> trimmed_control{
				u8_text.data(),
				static_cast<std::size_t>(trimmed_end - u8_text.data())};

			CHECK(trimmed_control ==
				std::basic_string_view<utf8_char_std>{
					reinterpret_cast<utf8_char_std*>(out_buff.data()),
					static_cast<std::size_t>(result.ptr - out_buff.data())});
		}
	};

	SECTION("utf8_to_utf16") {
		std::array<char16_t, 512> out_buff{ 0 };
		std::size_t char_num{ (std::numeric_limits<std::size_t>::max)() };
		const unsigned char* in_begin{ reinterpret_cast<const unsigned char*>(u8_text.data()) };
		auto result = fstlog::detail::utf8_to_utf16<utf8_char_lib, char16_t>(
			in_begin, in_begin + u8_text.size(),
			out_buff.data(), out_buff.data() + out_buff.size(),
			char_num);
		CHECK(result.ec == fstlog::error_code::none);
		CHECK(char_num == text_char_num);
		CHECK(u16_text == std::basic_string_view<char16_t>{
			out_buff.data(),
			static_cast<std::size_t>(result.ptr - out_buff.data())});
	};

	SECTION("utf8_to_utf32") {
		std::array<char32_t, 512> out_buff{ 0 };
		std::size_t char_num{ (std::numeric_limits<std::size_t>::max)() };
		const unsigned char* in_begin{ reinterpret_cast<const unsigned char*>(u8_text.data()) };
		auto result = fstlog::detail::utf8_to_utf32<utf8_char_lib, char32_t>(
			in_begin, in_begin + u8_text.size(),
			out_buff.data(), out_buff.data() + out_buff.size(),
			char_num);
		CHECK(result.ec == fstlog::error_code::none);
		CHECK(char_num == text_char_num);
		CHECK(u32_text == std::basic_string_view<char32_t>{
			out_buff.data(),
			static_cast<std::size_t>(result.ptr - out_buff.data())});
	};

	SECTION("utf8_to_utf32_invalid") {
		std::array<char32_t, 512> out_buff{ 0 };
		std::size_t char_num{ (std::numeric_limits<std::size_t>::max)() };
		std::size_t conv_length{ 102 };
		const unsigned char* const in_begin{ reinterpret_cast<const unsigned char*>(u8_text.data()) };
		const unsigned char* const in_end = in_begin + conv_length;
		const unsigned char* in_pos = in_begin;
		auto result = fstlog::detail::utf8_to_utf32<utf8_char_lib, char32_t>(
			in_pos, in_end,
			out_buff.data(), out_buff.data() + out_buff.size(),
			char_num);
		CHECK(result.ec == fstlog::error_code::input_bad);
		CHECK(in_pos == in_end - 1);
		const auto converted_char_num{ char_num };

		in_pos = in_begin;
		out_buff.fill(0);
		char_num = 1024;
		conv_length = 101;
		result = fstlog::detail::utf8_to_utf32<utf8_char_lib, char32_t>(
			in_pos, in_pos + conv_length,
			out_buff.data(), out_buff.data() + out_buff.size(),
			char_num);
		CHECK(result.ec == fstlog::error_code::none);
		CHECK(in_pos == in_begin + conv_length);
		CHECK(char_num == converted_char_num);
	};

	SECTION("utf32_to_utf8_invalid") {
		std::array<char32_t, 512> in_buff{ 0 };
		std::size_t conv_length{ 100 };
		memcpy(in_buff.data(), u32_text.data(), sizeof(char32_t) * conv_length);
		in_buff.at(conv_length - 1) = 0xffffffff;
		std::array<unsigned char, 512> out_buff{ 0 };
		std::size_t char_num{ (std::numeric_limits<std::size_t>::max)() };
		const unsigned char* const in_begin{ reinterpret_cast<const unsigned char*>(in_buff.data()) };
		const unsigned char* const in_end = in_begin + conv_length * sizeof(char32_t);
		const unsigned char* in_pos = in_begin;
		auto result = fstlog::detail::utf32_to_utf8<char32_t, unsigned char>(
			in_pos, in_end,
			out_buff.data(), out_buff.data() + out_buff.size(),
			char_num);
		CHECK(result.ec == fstlog::error_code::input_bad);
		CHECK(in_pos == in_end - sizeof(char32_t));
		const auto converted_char_num{ char_num };
		CHECK(converted_char_num == conv_length - 1);

		in_pos = in_begin;
		out_buff.fill(0);
		char_num = 1024;
		result = fstlog::detail::utf32_to_utf8<char32_t, unsigned char>(
			in_pos, in_end - sizeof(char32_t),
			out_buff.data(), out_buff.data() + out_buff.size(),
			char_num);
		CHECK(result.ec == fstlog::error_code::none);
		CHECK(in_pos == in_end - sizeof(char32_t));
		CHECK(char_num == converted_char_num);
	};

	SECTION("utf16_to_utf8_invalid") {
		std::array<char16_t, 512> in_buff{ 0 };
		std::size_t conv_length{ 100 };
		memcpy(in_buff.data(), u16_text.data(), sizeof(char16_t) * conv_length);
		in_buff.at(conv_length - 1) = 56319; //surrogate_high max
		std::array<unsigned char, 512> out_buff{ 0 };
		std::size_t char_num{ (std::numeric_limits<std::size_t>::max)() };
		const unsigned char* const in_begin{ reinterpret_cast<const unsigned char*>(in_buff.data()) };
		const unsigned char* const in_end = in_begin + conv_length * sizeof(char16_t);
		const unsigned char* in_pos = in_begin;
		auto result = fstlog::detail::utf16_to_utf8<char16_t, unsigned char>(
			in_pos, in_end,
			out_buff.data(), out_buff.data() + out_buff.size(),
			char_num);
		CHECK(result.ec == fstlog::error_code::input_bad);
		CHECK(in_pos == in_end - sizeof(char16_t));
		const auto converted_char_num{ char_num };
		CHECK(converted_char_num == conv_length - 1);

		in_pos = in_begin;
		out_buff.fill(0);
		char_num = 1024;
		result = fstlog::detail::utf16_to_utf8<char16_t, unsigned char>(
			in_pos, in_end - sizeof(char16_t),
			out_buff.data(), out_buff.data() + out_buff.size(),
			char_num);
		CHECK(result.ec == fstlog::error_code::none);
		CHECK(in_pos == in_end - sizeof(char16_t));
		CHECK(char_num == converted_char_num);
	};
}
