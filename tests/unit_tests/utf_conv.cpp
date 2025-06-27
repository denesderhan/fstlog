//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma execution_character_set("utf-8")
#include <cstring>
#include <limits>
#include <string_view>
#include <type_traits>
#include <array>
#include <vector>
#include <iostream>

#include <catch2/catch_all.hpp>

#include <detail/utf_conv.hpp>

using utf8_char_std = std::remove_const_t<std::remove_pointer_t<std::decay_t<decltype(u8"")>>>;
using utf8_char_lib = std::conditional<std::is_signed_v<utf8_char_std>, std::make_unsigned<utf8_char_std>::type, utf8_char_std>::type;

TEST_CASE("valid_utf_char") {
	SECTION("valid_utf32_char") {
		CHECK(fstlog::detail::valid_utf32_char((std::numeric_limits<std::int32_t>::min)()) == false);
		CHECK(fstlog::detail::valid_utf32_char(std::int32_t(-1)) == false);
		CHECK(fstlog::detail::valid_utf32_char(std::int32_t(0)) == true);
		CHECK(fstlog::detail::valid_utf32_char(std::int32_t(10)) == true);
		CHECK(fstlog::detail::valid_utf32_char((std::numeric_limits<std::int32_t>::max)()) == true);

		CHECK(fstlog::detail::valid_utf32_char((std::numeric_limits<std::uint32_t>::min)()) == true);
		CHECK(fstlog::detail::valid_utf32_char(std::uint32_t(10)) == true);
		CHECK(fstlog::detail::valid_utf32_char((std::numeric_limits<std::uint32_t>::max)()) == true);

		CHECK(fstlog::detail::valid_utf32_char((std::numeric_limits<std::int64_t>::min)()) == false);
		CHECK(fstlog::detail::valid_utf32_char(std::int64_t(-1)) == false);
		CHECK(fstlog::detail::valid_utf32_char(std::int64_t(0)) == true);
		CHECK(fstlog::detail::valid_utf32_char(std::int64_t(10)) == true);
		CHECK(fstlog::detail::valid_utf32_char(std::int64_t(0xFFFF'FFFFU)) == true);
		CHECK(fstlog::detail::valid_utf32_char(std::int64_t(0x1'0000'0000U)) == false);
		CHECK(fstlog::detail::valid_utf32_char((std::numeric_limits<std::int64_t>::max)()) == false);

		CHECK(fstlog::detail::valid_utf32_char((std::numeric_limits<std::uint64_t>::min)()) == true);
		CHECK(fstlog::detail::valid_utf32_char(std::uint64_t(10)) == true);
		CHECK(fstlog::detail::valid_utf32_char(std::uint64_t(0xFFFF'FFFFU)) == true);
		CHECK(fstlog::detail::valid_utf32_char(std::uint64_t(0x1'0000'0000U)) == false);
		CHECK(fstlog::detail::valid_utf32_char((std::numeric_limits<std::uint64_t>::max)()) == false);
	}

	SECTION("valid_utf16_char") {
		CHECK(fstlog::detail::valid_utf16_char((std::numeric_limits<std::int16_t>::min)()) == false);
		CHECK(fstlog::detail::valid_utf16_char(std::int16_t(-1)) == false);
		CHECK(fstlog::detail::valid_utf16_char(std::int16_t(0)) == true);
		CHECK(fstlog::detail::valid_utf16_char(std::int16_t(10)) == true);
		CHECK(fstlog::detail::valid_utf16_char((std::numeric_limits<std::int16_t>::max)()) == true);

		CHECK(fstlog::detail::valid_utf16_char((std::numeric_limits<std::uint16_t>::min)()) == true);
		CHECK(fstlog::detail::valid_utf16_char(std::uint16_t(10)) == true);
		CHECK(fstlog::detail::valid_utf16_char((std::numeric_limits<std::uint16_t>::max)()) == true);

		CHECK(fstlog::detail::valid_utf16_char((std::numeric_limits<std::int64_t>::min)()) == false);
		CHECK(fstlog::detail::valid_utf16_char(std::int64_t(-1)) == false);
		CHECK(fstlog::detail::valid_utf16_char(std::int64_t(0)) == true);
		CHECK(fstlog::detail::valid_utf16_char(std::int64_t(10)) == true);
		CHECK(fstlog::detail::valid_utf16_char(std::int64_t(0xFFFFU)) == true);
		CHECK(fstlog::detail::valid_utf16_char(std::int64_t(0x1'0000U)) == false);
		CHECK(fstlog::detail::valid_utf16_char((std::numeric_limits<std::int64_t>::max)()) == false);

		CHECK(fstlog::detail::valid_utf16_char((std::numeric_limits<std::uint64_t>::min)()) == true);
		CHECK(fstlog::detail::valid_utf16_char(std::uint64_t(10)) == true);
		CHECK(fstlog::detail::valid_utf16_char(std::uint64_t(0xFFFFU)) == true);
		CHECK(fstlog::detail::valid_utf16_char(std::uint64_t(0x1'0000U)) == false);
		CHECK(fstlog::detail::valid_utf16_char((std::numeric_limits<std::uint64_t>::max)()) == false);
	}

	SECTION("valid_utf8_char") {
		CHECK(fstlog::detail::valid_utf8_char((std::numeric_limits<std::int8_t>::min)()) == false);
		CHECK(fstlog::detail::valid_utf8_char(std::int8_t(-1)) == false);
		CHECK(fstlog::detail::valid_utf8_char(std::int8_t(0)) == true);
		CHECK(fstlog::detail::valid_utf8_char(std::int8_t(10)) == true);
		CHECK(fstlog::detail::valid_utf8_char((std::numeric_limits<std::int8_t>::max)()) == true);

		CHECK(fstlog::detail::valid_utf8_char((std::numeric_limits<std::uint8_t>::min)()) == true);
		CHECK(fstlog::detail::valid_utf8_char(std::uint8_t(10)) == true);
		CHECK(fstlog::detail::valid_utf8_char((std::numeric_limits<std::uint8_t>::max)()) == true);

		CHECK(fstlog::detail::valid_utf8_char((std::numeric_limits<std::int64_t>::min)()) == false);
		CHECK(fstlog::detail::valid_utf8_char(std::int64_t(-1)) == false);
		CHECK(fstlog::detail::valid_utf8_char(std::int64_t(0)) == true);
		CHECK(fstlog::detail::valid_utf8_char(std::int64_t(10)) == true);
		CHECK(fstlog::detail::valid_utf8_char(std::int64_t(0xFFU)) == true);
		CHECK(fstlog::detail::valid_utf8_char(std::int64_t(0x100U)) == false);
		CHECK(fstlog::detail::valid_utf8_char((std::numeric_limits<std::int64_t>::max)()) == false);

		CHECK(fstlog::detail::valid_utf8_char((std::numeric_limits<std::uint64_t>::min)()) == true);
		CHECK(fstlog::detail::valid_utf8_char(std::uint64_t(10)) == true);
		CHECK(fstlog::detail::valid_utf8_char(std::uint64_t(0xFFU)) == true);
		CHECK(fstlog::detail::valid_utf8_char(std::uint64_t(0x100U)) == false);
		CHECK(fstlog::detail::valid_utf8_char((std::numeric_limits<std::uint64_t>::max)()) == false);
	}
}

TEST_CASE("valid_utf_code_point") {
	CHECK(fstlog::detail::valid_utf_code_point(std::uint32_t{ 0 }) == true);
	CHECK(fstlog::detail::valid_utf_code_point(std::uint32_t{ 0xD7FF }) == true);
	CHECK(fstlog::detail::valid_utf_code_point(std::uint32_t{ 0xD800 }) == false);
	CHECK(fstlog::detail::valid_utf_code_point(std::uint32_t{ 0xD801 }) == false);
	CHECK(fstlog::detail::valid_utf_code_point(std::uint32_t{ 0xDFFE }) == false);
	CHECK(fstlog::detail::valid_utf_code_point(std::uint32_t{ 0xDFFF }) == false);
	CHECK(fstlog::detail::valid_utf_code_point(std::uint32_t{ 0xE000 }) == true);
	CHECK(fstlog::detail::valid_utf_code_point(std::uint32_t{ 0xE001 }) == true);
	CHECK(fstlog::detail::valid_utf_code_point(std::uint32_t{ 0x10000 }) == true);
	CHECK(fstlog::detail::valid_utf_code_point(std::uint32_t{ 0x10FFFF }) == true);
	CHECK(fstlog::detail::valid_utf_code_point(std::uint32_t{ 0x110000 }) == false);
	CHECK(fstlog::detail::valid_utf_code_point(std::uint32_t{ 0xFFFFFFFF }) == false);
}

TEST_CASE("safe_utf_code_point") {
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x0 }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0xF }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x1F }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x20 }) == true);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x7E }) == true);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x7F }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x80 }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x9F }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0xA0 }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0xA1 }) == true);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x02AF }) == true);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x02B0 }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x036F }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x0370 }) == true);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x06FF }) == true);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x0700 }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x08FF }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x0900 }) == true);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x09FE }) == true);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x09FF }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x0DFF }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x0E00 }) == true);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x10FF }) == true);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x1100 }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x2070 }) == true);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x20BF }) == true);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x20C0 }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x2FFF }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x3000 }) == true);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x30FF }) == true);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x3100 }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x3300 }) == true);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x9FFF }) == true);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0xA000 }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0xABFF }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0xAC00 }) == true);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0xD7AF }) == true);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0xD7B0 }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0xD7B1 }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0xFFFD }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0xFFFF }) == false);
	CHECK(fstlog::detail::safe_utf_code_point(std::uint32_t{ 0x10FFFF }) == false);
}

TEST_CASE("encode_escaped") {
	std::array<unsigned char, 16> buffer;
	
	SECTION("no_space_in_buffer") {
		auto result = fstlog::detail::encode_escaped(0, buffer.data(), buffer.data() + 5);
		CHECK(result.ec == fstlog::error_code::buff_full);
		CHECK(result.ptr == buffer.data());
		result = fstlog::detail::encode_escaped(0, buffer.data(), buffer.data() + 6);
		CHECK(result.ec == fstlog::error_code::none);
		CHECK(result.ptr == buffer.data() + 6);
		result = fstlog::detail::encode_escaped(0x10000, buffer.data(), buffer.data() + 9);
		CHECK(result.ec == fstlog::error_code::buff_full);
		CHECK(result.ptr == buffer.data());
		result = fstlog::detail::encode_escaped(0x10000, buffer.data(), buffer.data() + 10);
		CHECK(result.ec == fstlog::error_code::none);
		CHECK(result.ptr == buffer.data() + 10);
	}
	
	SECTION("regular") {
		auto data = GENERATE(
			std::make_tuple(std::uint32_t{ 0 }, "\\u0000"),
			std::make_tuple(std::uint32_t{ 0xFFFF }, "\\uFFFF"),
			std::make_tuple(std::uint32_t{ 0x10000 }, "\\U00010000"),
			std::make_tuple(std::uint32_t{ 0x10FFFF }, "\\U0010FFFF"),
			std::make_tuple(std::uint32_t{ 0x09ABCD }, "\\U0009ABCD"),
			std::make_tuple(std::uint32_t{ 0xABCD }, "\\uABCD")
		);
		buffer.fill('!');
		auto code_point = std::get<0>(data);
		std::string_view control = std::get<1>(data);

		auto result = fstlog::detail::encode_escaped(code_point, buffer.data(), buffer.data() + buffer.size());
		CHECK(result.ec == fstlog::error_code::none);
		const std::size_t len = code_point <= 0xFFFF ? 6 : 10;
		CHECK(result.ptr == buffer.data() + len);
		CHECK(std::string_view(reinterpret_cast<const char*>(buffer.data()), len) == control);
	}
}

TEST_CASE("encode_safe_utf8_char") {
	std::array<unsigned char, 16> buffer{ 0 };

	SECTION("no_space_in_buffer") {
		auto next_ptr = fstlog::detail::encode_safe_utf8_char(0, buffer.data(), buffer.data() + 5);
		CHECK(next_ptr == nullptr);
		next_ptr = fstlog::detail::encode_safe_utf8_char(0x10000, buffer.data(), buffer.data() + 9);
		CHECK(next_ptr == nullptr);
		next_ptr = fstlog::detail::encode_safe_utf8_char(0x00A9, buffer.data(), buffer.data() + 1);
		CHECK(next_ptr == nullptr);
	}

	SECTION("regular") {
		auto data = GENERATE(
			std::make_tuple(std::uint32_t{ 0 }, std::basic_string_view(u8"\\u0000")),
			std::make_tuple(std::uint32_t{ 0x00A9 }, std::basic_string_view(u8"©")),
			std::make_tuple(std::uint32_t{ 0xFFFD }, std::basic_string_view(u8"\\uFFFD")),
			std::make_tuple(std::uint32_t{ 'a' }, std::basic_string_view(u8"a")),
			std::make_tuple(std::uint32_t{ 0x10FFFF }, std::basic_string_view(u8"\\U0010FFFF"))
		);
		buffer.fill('!');
		auto code_point = std::get<0>(data);
		std::string_view control = { reinterpret_cast<const char*>(std::get<1>(data).data()), std::get<1>(data).size() };

		auto result = fstlog::detail::encode_safe_utf8_char(code_point, buffer.data(), buffer.data() + buffer.size());
		CHECK(result != nullptr);
		CHECK(result <= buffer.data() + buffer.size());
		const std::size_t len = result - buffer.data();
		CHECK(std::string_view(reinterpret_cast<const char*>(buffer.data()), len) == control);
	}
}

TEST_CASE("encode_utf16_char") {
	std::array<char16_t, 4> buffer{ 0 };

	SECTION("no_space_in_buffer") {
		auto next_ptr = fstlog::detail::encode_utf16_char(0, buffer.data(), buffer.data());
		CHECK(next_ptr == nullptr);
		next_ptr = fstlog::detail::encode_utf16_char(0x10000, buffer.data(), buffer.data() + 1);
		CHECK(next_ptr == nullptr);
	}

	SECTION("regular") {
		auto data = GENERATE(
			std::make_tuple(std::uint32_t{ 1 }, std::basic_string_view(u"\u0001")),
			std::make_tuple(std::uint32_t{ 0x00A9 }, std::basic_string_view(u"©")),
			std::make_tuple(std::uint32_t{ 0xFFFD }, std::basic_string_view(u"�")),
			std::make_tuple(std::uint32_t{ 'a' }, std::basic_string_view(u"a")),
			std::make_tuple(std::uint32_t{ 0x1F600 }, std::basic_string_view(u"😀"))
		);
		buffer.fill('!');
		auto code_point = std::get<0>(data);
		auto control = std::get<1>(data);

		auto result = fstlog::detail::encode_utf16_char(code_point, buffer.data(), buffer.data() + buffer.size());
		CHECK(result != nullptr);
		CHECK(result <= buffer.data() + buffer.size());
		const std::size_t len = result - buffer.data();
		CHECK(std::basic_string_view(buffer.data(), len) == control);
	}
}

TEST_CASE("decode_utf16_char") {
	std::array<char16_t, 4> buffer{ 0 };
	const unsigned char* begin = reinterpret_cast<const unsigned char*>(buffer.data());

	SECTION("valid_1") {
		buffer[0] = 'a';
		auto ptr = begin;
		auto code_point = fstlog::detail::decode_utf16_char<char16_t>(ptr, ptr + sizeof(char16_t));
		CHECK(ptr - begin == sizeof(char16_t));
		CHECK(code_point == 'a');
		buffer[0] = 0x20AC;
		ptr = begin;
		code_point = fstlog::detail::decode_utf16_char<char16_t>(ptr, ptr + sizeof(char16_t));
		CHECK(ptr - begin == sizeof(char16_t));
		CHECK(code_point == 0x20AC);
	}
	SECTION("missing_surrogate") {
		buffer[0] = 0xD83D;// \ude00;
		auto ptr = begin;
		auto code_point = fstlog::detail::decode_utf16_char<char16_t>(ptr, ptr + sizeof(char16_t));
		CHECK(ptr - begin == sizeof(char16_t));
		CHECK(code_point == 0xFFFD);
	}
	SECTION("invalid_1st_surrogate") {
		buffer[0] = 0xDE00;
		buffer[1] = 0xDE00;
		auto ptr = begin;
		auto code_point = fstlog::detail::decode_utf16_char<char16_t>(ptr, ptr + 2 * sizeof(char16_t));
		CHECK(ptr - begin == sizeof(char16_t));
		CHECK(code_point == 0xFFFD);
	}
	SECTION("invalid_2nd_surrogate") {
		buffer[0] = 0xD83D;
		buffer[1] = 0x0;
		auto ptr = begin;
		auto code_point = fstlog::detail::decode_utf16_char<char16_t>(ptr, ptr + 2 * sizeof(char16_t));
		CHECK(ptr - begin == sizeof(char16_t));
		CHECK(code_point == 0xFFFD);

		buffer[0] = 0xD83D;
		buffer[1] = 0xD83D;
		ptr = begin;
		code_point = fstlog::detail::decode_utf16_char<char16_t>(ptr, ptr + 2 * sizeof(char16_t));
		CHECK(ptr - begin == sizeof(char16_t));
		CHECK(code_point == 0xFFFD);

		buffer[0] = 0xD83D;
		buffer[1] = 0xFFFF;
		ptr = begin;
		code_point = fstlog::detail::decode_utf16_char<char16_t>(ptr, ptr + 2 * sizeof(char16_t));
		CHECK(ptr - begin == sizeof(char16_t));
		CHECK(code_point == 0xFFFD);
	}
	SECTION("invalid_1_2_surrogate") {
		buffer[0] = 0xDE00;
		buffer[1] = 0xD83D;
		auto ptr = begin;
		auto code_point = fstlog::detail::decode_utf16_char<char16_t>(ptr, ptr + 2 * sizeof(char16_t));
		CHECK(ptr - begin == sizeof(char16_t));
		CHECK(code_point == 0xFFFD);

		buffer[0] = 0xDC00;
		buffer[1] = 0xDB7F;
		ptr = begin;
		code_point = fstlog::detail::decode_utf16_char<char16_t>(ptr, ptr + 2 * sizeof(char16_t));
		CHECK(ptr - begin == sizeof(char16_t));
		CHECK(code_point == 0xFFFD);
	}
	SECTION("valid_surrogates") {
		buffer[0] = 0xD83D;
		buffer[1] = 0xDE00;
		auto ptr = begin;
		auto code_point = fstlog::detail::decode_utf16_char<char16_t>(ptr, ptr + 2 * sizeof(char16_t));
		CHECK(ptr - begin == 2 * sizeof(char16_t));
		CHECK(code_point == 0x1F600);

		buffer[0] = 0xDB7F;
		buffer[1] = 0xDC00;
		ptr = begin;
		code_point = fstlog::detail::decode_utf16_char<char16_t>(ptr, ptr + 2 * sizeof(char16_t));
		CHECK(ptr - begin == 2 * sizeof(char16_t));
		CHECK(code_point == 0xEFC00);
	}
}

TEST_CASE("utfX_to_utfY") {
	constexpr std::basic_string_view<utf8_char_std> u8_text{
		u8"UTF encoding is a standard for encoding characters in the Unicode character set."
		u8"L'encodage UTF est une norme pour l'encodage des caractères dans l'ensemble de caractères Unicode."
		u8"UTF kódování je standard pro kódování znaků v sadě znaků Unicode."
		u8"Kodowanie UTF jest standardem kodowania znaków w zestawie znaków Unicode."
		u8"UTF-kodning er en standard for kodning af tegn i Unicode-tegnsættet."
		u8"Η κωδικοποίηση UTF είναι ένα πρότυπο για την κωδικοποίηση χαρακτήρων στο σύνολο χαρακτήρων Unicode."
		u8"La codificación UTF es un estándar para codificar caracteres en el conjunto de caracteres Unicode."
		u8"Кодировка UTF является стандартом для кодирования символов в наборе символов Unicode."
		u8"ترميز UTF هو معيار لترميز الأحرف في مجموعة أحرف يونيكود."
		u8"UTF एन्कोडिंग यूनिकोड कैरेक्टर सेट में कैरेक्टर को एन्कोड करने के लिए एक मानक है।"
		u8"การเข้ารหัส UTF เป็นมาตรฐานสำหรับการเข้ารหัสตัวอักษรในชุดตัวอักษร Unicode."
		u8"UTF კოდირება არის სტანდარტი სიმბოლოების კოდირებისთვის Unicode სიმბოლოების ნაკრარში."
		u8"UTFエンコーディングは、Unicode文字セット内の文字をエンコードするための標準です。"
		u8"UTF 编码者, Unicode 字符集字符编码之准也。"
		u8"UTF 인코딩은 유니코드 문자 집합에서 문자를 인코딩하는 표준입니다."
	};
	constexpr std::basic_string_view <char16_t> u16_text{
		u"UTF encoding is a standard for encoding characters in the Unicode character set."
		u"L'encodage UTF est une norme pour l'encodage des caractères dans l'ensemble de caractères Unicode."
		u"UTF kódování je standard pro kódování znaků v sadě znaků Unicode."
		u"Kodowanie UTF jest standardem kodowania znaków w zestawie znaków Unicode."
		u"UTF-kodning er en standard for kodning af tegn i Unicode-tegnsættet."
		u"Η κωδικοποίηση UTF είναι ένα πρότυπο για την κωδικοποίηση χαρακτήρων στο σύνολο χαρακτήρων Unicode."
		u"La codificación UTF es un estándar para codificar caracteres en el conjunto de caracteres Unicode."
		u"Кодировка UTF является стандартом для кодирования символов в наборе символов Unicode."
		u"ترميز UTF هو معيار لترميز الأحرف في مجموعة أحرف يونيكود."
		u"UTF एन्कोडिंग यूनिकोड कैरेक्टर सेट में कैरेक्टर को एन्कोड करने के लिए एक मानक है।"
		u"การเข้ารหัส UTF เป็นมาตรฐานสำหรับการเข้ารหัสตัวอักษรในชุดตัวอักษร Unicode."
		u"UTF კოდირება არის სტანდარტი სიმბოლოების კოდირებისთვის Unicode სიმბოლოების ნაკრარში."
		u"UTFエンコーディングは、Unicode文字セット内の文字をエンコードするための標準です。"
		u"UTF 编码者, Unicode 字符集字符编码之准也。"
		u"UTF 인코딩은 유니코드 문자 집합에서 문자를 인코딩하는 표준입니다."
	};

	constexpr std::basic_string_view <char32_t> u32_text{
		U"UTF encoding is a standard for encoding characters in the Unicode character set."
		U"L'encodage UTF est une norme pour l'encodage des caractères dans l'ensemble de caractères Unicode."
		U"UTF kódování je standard pro kódování znaků v sadě znaků Unicode."
		U"Kodowanie UTF jest standardem kodowania znaków w zestawie znaków Unicode."
		U"UTF-kodning er en standard for kodning af tegn i Unicode-tegnsættet."
		U"Η κωδικοποίηση UTF είναι ένα πρότυπο για την κωδικοποίηση χαρακτήρων στο σύνολο χαρακτήρων Unicode."
		U"La codificación UTF es un estándar para codificar caracteres en el conjunto de caracteres Unicode."
		U"Кодировка UTF является стандартом для кодирования символов в наборе символов Unicode."
		U"ترميز UTF هو معيار لترميز الأحرف في مجموعة أحرف يونيكود."
		U"UTF एन्कोडिंग यूनिकोड कैरेक्टर सेट में कैरेक्टर को एन्कोड करने के लिए एक मानक है।"
		U"การเข้ารหัส UTF เป็นมาตรฐานสำหรับการเข้ารหัสตัวอักษรในชุดตัวอักษร Unicode."
		U"UTF კოდირება არის სტანდარტი სიმბოლოების კოდირებისთვის Unicode სიმბოლოების ნაკრარში."
		U"UTFエンコーディングは、Unicode文字セット内の文字をエンコードするための標準です。"
		U"UTF 编码者, Unicode 字符集字符编码之准也。"
		U"UTF 인코딩은 유니코드 문자 집합에서 문자를 인코딩하는 표준입니다."
	};
	constexpr auto text_char_num{ u32_text.size() };

	SECTION("utf32_to_utf8") {
		std::size_t char_num = GENERATE(0, 530, 1071, (std::numeric_limits<std::size_t>::max)());
		CAPTURE(char_num);
		{
			std::vector<utf8_char_lib> out_buff(4096, 0);
			const auto wanted_char_num{ std::min(char_num, text_char_num) };
			unsigned char* out_data = reinterpret_cast<unsigned char*>(out_buff.data());
			unsigned char const* out_end = out_data + out_buff.size();
			const unsigned char* in_begin{ reinterpret_cast<const unsigned char*>(u32_text.data()) };
			const auto result = fstlog::detail::utf32_to_utf8<char32_t>(
				in_begin, in_begin + (u32_text.size() * sizeof(char32_t)),
				out_data, out_end,
				char_num);
			CHECK(result.ec == fstlog::error_code::none);
			CHECK(char_num == wanted_char_num);

			const auto wanted_trim{ char_num };
			auto trimmed_num{ wanted_trim };
			const auto trimmed_len = fstlog::detail::utf8_str_trim(
				u8_text.data(),
				u8_text.data() + u8_text.size(),
				trimmed_num);
			CHECK(wanted_trim == trimmed_num);

			const std::basic_string_view<utf8_char_std> trimmed_control{
				u8_text.data(),
				trimmed_len
			};

			CHECK(trimmed_control ==
				std::basic_string_view<utf8_char_std>{
				out_buff.data(),
					static_cast<std::size_t>(reinterpret_cast<utf8_char_std*>(result.ptr) - out_buff.data())
			});
		}
	};

	SECTION("utf16_to_utf8") {
		std::vector<utf8_char_lib> out_buff(4096, 0);
		unsigned char* out_data = reinterpret_cast<unsigned char*>(out_buff.data());
		unsigned char const* out_end = out_data + out_buff.size();
		std::size_t char_num = GENERATE(0, 530, 1071, (std::numeric_limits<std::size_t>::max)());
		CAPTURE(char_num);
		{
			const auto wanted_charnum{ char_num };
			const unsigned char* in_begin{ reinterpret_cast<const unsigned char*>(u16_text.data()) };
			const auto result = fstlog::detail::utf16_to_utf8<char16_t>(
				in_begin, in_begin + (u16_text.size() * sizeof(char16_t)),
				out_data, out_end,
				char_num);
			CHECK(result.ec == fstlog::error_code::none);
			if(wanted_charnum == (std::numeric_limits<std::size_t>::max)())
				CHECK(char_num == text_char_num);
			else CHECK(char_num == wanted_charnum);

			const auto wanted_trim{ char_num };
			auto trimmed_num{ wanted_trim };
			const auto trimmed_len = fstlog::detail::utf8_str_trim(
					u8_text.data(),
					u8_text.data() + u8_text.size(),
					trimmed_num);
			CHECK(wanted_trim == trimmed_num);

			const std::basic_string_view<utf8_char_std> trimmed_control{
				u8_text.data(),
				trimmed_len};

			CHECK(trimmed_control ==
				std::basic_string_view<utf8_char_std>{
					reinterpret_cast<utf8_char_std*>(out_data),
					static_cast<std::size_t>(result.ptr - out_data)});
		}
	};

	SECTION("utf8_to_utf8") {
		std::vector<utf8_char_lib> out_buff(4096, 0);
		unsigned char* out_data = reinterpret_cast<unsigned char*>(out_buff.data());
		unsigned char const* out_end = out_data + out_buff.size();
		std::size_t char_num = GENERATE(0, 530, 1071, (std::numeric_limits<std::size_t>::max)());
		CAPTURE(char_num);
		{
			const auto wanted_charnum{ char_num };
			const unsigned char* in_begin{ reinterpret_cast<const unsigned char*>(u8_text.data()) };
			const auto result = fstlog::detail::utf8_to_utf8<utf8_char_lib>(
				in_begin, in_begin + (u8_text.size()),
				out_data, out_end,
				char_num);
			CHECK(result.ec == fstlog::error_code::none);
			if (wanted_charnum == (std::numeric_limits<std::size_t>::max)())
				CHECK(char_num == text_char_num);
			else CHECK(char_num == wanted_charnum);

			const auto wanted_trim{ char_num };
			auto trimmed_num{ wanted_trim };
			const auto trimmed_len = fstlog::detail::utf8_str_trim(
				u8_text.data(),
				u8_text.data() + u8_text.size(),
				trimmed_num);
			CHECK(wanted_trim == trimmed_num);

			const std::basic_string_view<utf8_char_std> trimmed_control{
				u8_text.data(),
				trimmed_len };

			CHECK(trimmed_control ==
				std::basic_string_view<utf8_char_std>{
				reinterpret_cast<utf8_char_std*>(out_data),
					static_cast<std::size_t>(result.ptr - out_data)});
		}
	};

	SECTION("utf32_to_utf8_irregular") {
		auto test_data = GENERATE(
			std::make_tuple(std::vector<char32_t>{'S', 'u', 'r', 'r', 'o', 'g', 'a', 't', 'e', ':', 0xD800, 0xDA00, 0xDFFF}, std::basic_string_view<utf8_char_lib>{u8"Surrogate:\\uFFFD\\uFFFD\\uFFFD"}),
			std::make_tuple(std::vector<char32_t>{'O', 'u', 't', ':', 0x00110000, 0x001FFFFF, 0xFFFFFFFF}, std::basic_string_view<utf8_char_lib>{u8"Out:\\uFFFD\\uFFFD\\uFFFD"}),
			std::make_tuple(std::vector<char32_t>{'C', '0', ':', 0x0000, 0x000A, 0x000C, 0x001F, 0x007F}, std::basic_string_view<utf8_char_lib>{u8"C0:\\u0000\\u000A\\u000C\\u001F\\u007F"}),
			std::make_tuple(std::vector<char32_t>{'C', '1', ':', 0x0080, 0x008C, 0x009F}, std::basic_string_view<utf8_char_lib>{u8"C1:\\u0080\\u008C\\u009F"}),
			std::make_tuple(std::vector<char32_t>{'U', 'n', 's', 'a', 'f', 'e', ':', 0x02B0, 0x20D0, 0x0001F600, 0x000E01EF}, std::basic_string_view<utf8_char_lib>{u8"Unsafe:\\u02B0\\u20D0\\U0001F600\\U000E01EF"})
		);
		
		auto control_txt = std::get<1>(test_data);
		auto input = std::get<0>(test_data);
		std::vector<utf8_char_lib> out_buff(2048, 0);
		unsigned char* out_data = reinterpret_cast<unsigned char*>(out_buff.data());
		unsigned char const* out_end = out_data + out_buff.size();
		const unsigned char* in_begin{ reinterpret_cast<const unsigned char*>(input.data()) };
		auto in_end = in_begin + sizeof(char32_t) * input.size();
		std::size_t char_num = (std::numeric_limits<std::size_t>::max)();
		const auto result = fstlog::detail::utf32_to_utf8<char32_t>(
			in_begin, in_end,
			out_data, out_end,
			char_num);
		CAPTURE(reinterpret_cast<const char*>(out_data));
		CHECK(result.ec == fstlog::error_code::none);
		std::basic_string_view < utf8_char_lib> str_out{ 
			out_buff.data(), 
			static_cast<std::size_t>(result.ptr - out_data)};
		CHECK(control_txt == str_out);
	};

	SECTION("utf16_to_utf8_irregular") {
		auto test_data = GENERATE(
			std::make_tuple(std::vector<char16_t>{'S', 'u', 'r', 'r', 'o', 'g', 'a', 't', 'e', ':', 0xD800, '_', 0xDA00, '_', 0xDFFF}, std::basic_string_view<utf8_char_lib>{u8"Surrogate:\\uFFFD_\\uFFFD_\\uFFFD"}),
			std::make_tuple(std::vector<char16_t>{'C', '0', ':', 0x0000, 0x000A, 0x000C, 0x001F, 0x007F}, std::basic_string_view<utf8_char_lib>{u8"C0:\\u0000\\u000A\\u000C\\u001F\\u007F"}),
			std::make_tuple(std::vector<char16_t>{'C', '1', ':', 0x0080, 0x008C, 0x009F}, std::basic_string_view<utf8_char_lib>{u8"C1:\\u0080\\u008C\\u009F"}),
			std::make_tuple(std::vector<char16_t>{'U', 'n', 's', 'a', 'f', 'e', ':', 0x02B0, 0x20D0, 0xd83d, 0xde00, 0xd884, 0xdf50}, std::basic_string_view<utf8_char_lib>{u8"Unsafe:\\u02B0\\u20D0\\U0001F600\\U00031350"})
		);

		auto control_txt = std::get<1>(test_data);
		auto input = std::get<0>(test_data);
		std::vector<utf8_char_lib> out_buff(2048, 0);
		unsigned char* out_data = reinterpret_cast<unsigned char*>(out_buff.data());
		unsigned char const* out_end = out_data + out_buff.size();
		const unsigned char* in_begin{ reinterpret_cast<const unsigned char*>(input.data()) };
		auto in_end = in_begin + sizeof(char16_t) * input.size();
		std::size_t char_num = (std::numeric_limits<std::size_t>::max)();
		const auto result = fstlog::detail::utf16_to_utf8<char16_t>(
			in_begin, in_end,
			out_data, out_end,
			char_num);
		CAPTURE(reinterpret_cast<const char*>(out_data));
		CHECK(result.ec == fstlog::error_code::none);
		std::basic_string_view < utf8_char_lib> str_out{
			out_buff.data(),
			static_cast<std::size_t>(result.ptr - out_data) };
		CHECK(control_txt == str_out);
	};

	SECTION("utf8_to_utf8_irregular") {
		auto test_data = GENERATE(
			std::make_tuple(std::vector<utf8_char_lib>{'S', 'u', 'r', 'r', 'o', 'g', 'a', 't', 'e', ':', 0xED, 0xA0, 0x80, 0xED, 0xA8, 0x80, 0xED, 0xBF, 0xBF}, std::basic_string_view<utf8_char_lib>{u8"Surrogate:\\uFFFD\\uFFFD\\uFFFD"}),
			std::make_tuple(std::vector<utf8_char_lib>{'C', '0', ':', 0x0000, 0x000A, 0x000C, 0x001F, 0x007F}, std::basic_string_view<utf8_char_lib>{u8"C0:\\u0000\\u000A\\u000C\\u001F\\u007F"}),
			std::make_tuple(std::vector<utf8_char_lib>{'C', '1', ':', 0xC2, 0x80, 0xC2, 0x8C, 0xC2, 0x9F}, std::basic_string_view<utf8_char_lib>{u8"C1:\\u0080\\u008C\\u009F"}),
			std::make_tuple(std::vector<utf8_char_lib>{'U', 'n', 's', 'a', 'f', 'e', ':', 0xCA, 0xB0, 0xE2, 0x83, 0x90, 0xF0, 0x9F, 0x98, 0x80, 0xF0, 0xB1, 0x8D, 0x90}, std::basic_string_view<utf8_char_lib>{u8"Unsafe:\\u02B0\\u20D0\\U0001F600\\U00031350"})
		);

		auto control_txt = std::get<1>(test_data);
		auto input = std::get<0>(test_data);
		std::vector<utf8_char_lib> out_buff(2048, 0);
		unsigned char* out_data = reinterpret_cast<unsigned char*>(out_buff.data());
		unsigned char const* out_end = out_data + out_buff.size();
		const unsigned char* in_begin{ reinterpret_cast<const unsigned char*>(input.data()) };
		auto in_end = in_begin + input.size();
		std::size_t char_num = (std::numeric_limits<std::size_t>::max)();
		const auto result = fstlog::detail::utf8_to_utf8<utf8_char_lib>(
			in_begin, in_end,
			out_data, out_end,
			char_num);
		CAPTURE(reinterpret_cast<const char*>(out_data));
		CHECK(result.ec == fstlog::error_code::none);
		std::basic_string_view < utf8_char_lib> str_out{
			out_buff.data(),
			static_cast<std::size_t>(result.ptr - out_data) };
		CHECK(control_txt == str_out);
	};

	SECTION("utf32_to_utf8_no_space_in_buffer") {
		std::basic_string_view input{U"UTF 编码者"};
		std::array<unsigned char, 9> buffer;
		buffer.fill('!');
		unsigned char* out_data = buffer.data();
		unsigned char const* out_end = out_data + buffer.size();
		const unsigned char* in_begin{ reinterpret_cast<const unsigned char*>(input.data()) };
		auto in_ptr = in_begin;
		auto in_end = in_begin + sizeof(char32_t) * input.size();
		std::size_t char_num = (std::numeric_limits<std::size_t>::max)();
		const auto result = fstlog::detail::utf32_to_utf8<char32_t>(
			in_ptr, in_end,
			out_data, out_end,
			char_num);
		CHECK(result.ec == fstlog::error_code::buff_full);
		CHECK(in_ptr == in_begin + 5 * sizeof(char32_t));
		CHECK(char_num == 5);
		CHECK(result.ptr == out_data + 7);
		std::basic_string_view control{ u8"UTF 编" };
		CHECK(std::basic_string_view(reinterpret_cast<utf8_char_lib*>(buffer.data()), 7) == control);
	}

	SECTION("utf16_to_utf8_no_space_in_buffer") {
		std::basic_string_view input{ u"UTF 编码者" };
		std::array<unsigned char, 9> buffer;
		buffer.fill('!');
		unsigned char* out_data = buffer.data();
		unsigned char const* out_end = out_data + buffer.size();
		const unsigned char* in_begin{ reinterpret_cast<const unsigned char*>(input.data()) };
		auto in_ptr = in_begin;
		auto in_end = in_begin + sizeof(char16_t) * input.size();
		std::size_t char_num = (std::numeric_limits<std::size_t>::max)();
		const auto result = fstlog::detail::utf32_to_utf8<char16_t>(
			in_ptr, in_end,
			out_data, out_end,
			char_num);
		CHECK(result.ec == fstlog::error_code::buff_full);
		CHECK(in_ptr == in_begin + 5 * sizeof(char16_t));
		CHECK(char_num == 5);
		CHECK(result.ptr == out_data + 7);
		std::basic_string_view control{ u8"UTF 编" };
		CHECK(std::basic_string_view(reinterpret_cast<utf8_char_lib*>(buffer.data()), 7) == control);
	}

	SECTION("utf8_to_utf8_no_space_in_buffer") {
		std::basic_string_view input{ u8"UTF 编码者" };
		std::array<unsigned char, 9> buffer;
		buffer.fill('!');
		unsigned char* out_data = buffer.data();
		unsigned char const* out_end = out_data + buffer.size();
		const unsigned char* in_begin{ reinterpret_cast<const unsigned char*>(input.data()) };
		auto in_ptr = in_begin;
		auto in_end = in_begin + input.size();
		std::size_t char_num = (std::numeric_limits<std::size_t>::max)();
		const auto result = fstlog::detail::utf8_to_utf8<utf8_char_lib>(
			in_ptr, in_end,
			out_data, out_end,
			char_num);
		CHECK(result.ec == fstlog::error_code::buff_full);
		CHECK(in_ptr == in_begin + 7);
		CHECK(char_num == 5);
		CHECK(result.ptr == out_data + 7);
		std::basic_string_view control{ u8"UTF 编" };
		CHECK(std::basic_string_view(reinterpret_cast<utf8_char_lib*>(buffer.data()), 7) == control);
	}
}

TEST_CASE("decode_utf8_char_benchmark", "[.][benchmark]") {
	auto input = GENERATE(
		u8"UTF encoding is a standard for encoding characters in the Unicode character set.",
		u8"L'encodage UTF est une norme pour l'encodage des caractères dans l'ensemble de caractères Unicode.",
		u8"UTF kódování je standard pro kódování znaků v sadě znaků Unicode.",
		u8"Kodowanie UTF jest standardem kodowania znaków w zestawie znaków Unicode.",
		u8"UTF-kodning er en standard for kodning af tegn i Unicode-tegnsættet.",
		u8"Η κωδικοποίηση UTF είναι ένα πρότυπο για την κωδικοποίηση χαρακτήρων στο σύνολο χαρακτήρων Unicode.",
		u8"La codificación UTF es un estándar para codificar caracteres en el conjunto de caracteres Unicode.",
		u8"Кодировка UTF является стандартом для кодирования символов в наборе символов Unicode.",
		u8"ترميز UTF هو معيار لترميز الأحرف في مجموعة أحرف يونيكود.",
		u8"UTF एन्कोडिंग यूनिकोड कैरेक्टर सेट में कैरेक्टर को एन्कोड करने के लिए एक मानक है।",
		u8"การเข้ารหัส UTF เป็นมาตรฐานสำหรับการเข้ารหัสตัวอักษรในชุดตัวอักษร Unicode.",
		u8"UTF კოდირება არის სტანდარტი სიმბოლოების კოდირებისთვის Unicode სიმბოლოების ნაკრარში.",
		u8"UTFエンコーディングは、Unicode文字セット内の文字をエンコードするための標準です。",
		u8"UTF 编码者, Unicode 字符集字符编码之准也。",
		u8"UTF 인코딩은 유니코드 문자 집합에서 문자를 인코딩하는 표준입니다."
	);

	DYNAMIC_SECTION("input_" << reinterpret_cast<const char*>(input)) {
		const unsigned char* const begin = reinterpret_cast<const unsigned char*>(input);
		const unsigned char* const end = begin + strlen(reinterpret_cast<const char*>(input));
		BENCHMARK_ADVANCED("decode_utf8_char")(Catch::Benchmark::Chronometer meter) {
			meter.measure([begin, end] {
				std::size_t res = 0;
				auto pos = begin;
				while (pos < end) {
					res += fstlog::detail::decode_utf8_char(pos, end);
				}
				return res;
			});
		};
	};

};

TEST_CASE("decode_utf8_char") {

	SECTION("one_byte") {
		// valid encoded ASCII
		for (unsigned char c = 0x00; c < 0x80; c++) {
			unsigned char ascii = c;
			const unsigned char*  ascii_ptr = &ascii;
			CHECK(c == fstlog::detail::decode_utf8_char(ascii_ptr, ascii_ptr + 1));
			CHECK(ascii_ptr == &ascii + 1);
		}
		// invalid encoding
		for (unsigned char c = 0xFF; c >= 0x80; c--) {
			unsigned char ascii = c;
			const unsigned char* ascii_ptr = &ascii;
			CHECK(0xFFFD == fstlog::detail::decode_utf8_char(ascii_ptr, ascii_ptr + 1));
			CHECK(ascii_ptr == &ascii + 1);
		}
	}

	SECTION("two_byte") {
		// 0b110x'xxxx, 0b10yy'yyyy
		auto test = GENERATE(
			// valid encoded
			std::make_tuple(std::array<unsigned char, 2>{ 0b1100'0010, 0b1000'0000}, 2, 2, 0x80U),
			std::make_tuple(std::array<unsigned char, 2>{ 0b1101'0000, 0b1010'0000}, 2, 2, 0x420U),
			std::make_tuple(std::array<unsigned char, 2>{ 0b1101'1111, 0b1011'1111}, 2, 2, 0x7FFU),
			// overlong encoded
			std::make_tuple(std::array<unsigned char, 2>{ 0b1100'0000, 0b1000'0000}, 2, 2, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 2>{ 0b1100'0001, 0b1011'1111}, 2, 2, 0xFFFDU),
			// invalid first byte
			std::make_tuple(std::array<unsigned char, 2>{ 0b1001'0000, 0b1010'0000}, 2, 1, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 2>{ 0b1011'1111, 0b1011'1111}, 2, 1, 0xFFFDU),
			// invalid second byte
			std::make_tuple(std::array<unsigned char, 2>{ 0b1100'0010, 0b1100'0000}, 2, 1, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 2>{ 0b1101'0000, 0b0010'0000}, 2, 1, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 2>{ 0b1101'1111, 0b0111'1111}, 2, 1, 0xFFFDU),
			// invalid first, second byte
			std::make_tuple(std::array<unsigned char, 2>{ 0b1110'0010, 0b1100'0000}, 2, 1, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 2>{ 0b1001'0000, 0b0010'0000}, 2, 1, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 2>{ 0b1011'1111, 0b0111'1111}, 2, 1, 0xFFFDU),
			// missing second byte
			std::make_tuple(std::array<unsigned char, 2>{ 0b1100'0010, 0b1000'0000}, 1, 1, 0xFFFDU)
		);

		unsigned char const* begin = std::get<0>(test).data();
		unsigned char const* const end = begin + std::get<1>(test);
		unsigned char const* const begin_control = begin + std::get<2>(test);
		std::uint32_t const codep_control = std::get<3>(test);
		CHECK(codep_control == fstlog::detail::decode_utf8_char(begin, end));
		CHECK(begin_control == begin);
	}

	SECTION("three_byte") {
		// 0b1110'xxxx, 0b10yy'yyyy, 0b10zz'zzzz
		auto test = GENERATE(
			// valid encoded
			std::make_tuple(std::array<unsigned char, 3>{ 0b1110'0000, 0b1010'0000, 0b1000'0000}, 3, 3, 0x800U),
			std::make_tuple(std::array<unsigned char, 3>{ 0b1110'0000, 0b1010'0000, 0b1010'0000}, 3, 3, 0x820U),
			std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1111, 0b1011'1111, 0b1011'1111}, 3, 3, 0xFFFFU),
			std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1111, 0b1011'1111, 0b1011'1101}, 3, 3, 0xFFFDU),
			// overlong encoded
			std::make_tuple(std::array<unsigned char, 3>{ 0b1110'0000, 0b1000'0000, 0b1000'0000}, 3, 3, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 3>{ 0b1110'0000, 0b1001'1111, 0b1011'1111}, 3, 3, 0xFFFDU),
			// invalid first byte
			std::make_tuple(std::array<unsigned char, 3>{ 0b1000'0000, 0b1010'0000, 0b1000'0000}, 3, 1, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 3>{ 0b1001'0000, 0b1010'0000, 0b1010'0000}, 3, 1, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 3>{ 0b1010'1111, 0b1011'1111, 0b1011'1111}, 3, 1, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 3>{ 0b1000'0000, 0b1010'0000, 0b1000'0000}, 3, 1, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 3>{ 0b1011'0000, 0b1010'0000, 0b1010'0000}, 3, 1, 0xFFFDU),
			// invalid second byte
			std::make_tuple(std::array<unsigned char, 3>{ 0b1110'0000, 0b1110'0000, 0b1000'0000}, 3, 1, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 3>{ 0b1110'0000, 0b0110'0000, 0b1010'0000}, 3, 1, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1111, 0b0011'1111, 0b1011'1111}, 3, 1, 0xFFFDU),
			// invalid third byte
			std::make_tuple(std::array<unsigned char, 3>{ 0b1110'0000, 0b1010'0000, 0b1100'0000}, 3, 2, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 3>{ 0b1110'0000, 0b1010'0000, 0b0010'0000}, 3, 2, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1111, 0b1011'1111, 0b0011'1111}, 3, 2, 0xFFFDU),
			// missing second byte
			std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1111, 0, 0}, 1, 1, 0xFFFDU),
			// missing third byte
			std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1111, 0b1011'1111, 0}, 2, 1, 0xFFFDU),
			// surrogate range
			std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1101, 0b1010'0000, 0b1000'0000}, 3, 3, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1101, 0b1011'0000, 0b1000'0000}, 3, 3, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1101, 0b1011'1111, 0b1011'1111}, 3, 3, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1110, 0b1000'0000, 0b1000'0000}, 3, 3, 0xE000U)
		);
		
		unsigned char const* begin = std::get<0>(test).data();
		unsigned char const* const end = begin + std::get<1>(test);
		unsigned char const* const begin_control = begin + std::get<2>(test);
		std::uint32_t const codep_control = std::get<3>(test);
		CHECK(codep_control == fstlog::detail::decode_utf8_char(begin, end));
		CHECK(begin_control == begin);
	}
	
	SECTION("four_byte") {
		// 0b1110'xxxx, 0b10yy'yyyy, 0b10zz'zzzz, 0b10aa'aaaa
		auto test = GENERATE(
			// valid encoded
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1001'0000, 0b1000'0000, 0b1000'0000}, 4, 4, 0x10000U),
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1011'1111, 0b1011'1111, 0b1011'1111}, 4, 4, 0x3FFFFU),
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0100, 0b1000'1111, 0b1011'1111, 0b1011'1111}, 4, 4, 0x10FFFFU),
			// exceeding max
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0101, 0b1000'0000, 0b1000'0000, 0b1000'0000}, 4, 4, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0111, 0b1011'1111, 0b1011'1111, 0b1011'1111}, 4, 4, 0xFFFDU),
			// overlong encoded
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1000'0000, 0b1000'0000, 0b1000'0000}, 4, 4, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1000'1111, 0b1011'1111, 0b1011'1111}, 4, 4, 0xFFFDU),
			// invalid first byte
			std::make_tuple(std::array<unsigned char, 4>{ 0b1011'0000, 0b1001'0000, 0b1000'0000, 0b1000'0000}, 4, 1, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 4>{ 0b1001'0000, 0b1011'1111, 0b1011'1111, 0b1011'1111}, 4, 1, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 4>{ 0b1010'0100, 0b1000'1111, 0b1011'1111, 0b1011'1111}, 4, 1, 0xFFFDU),
			// invalid second byte
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b0001'0000, 0b1000'0000, 0b1000'0000}, 4, 1, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1111'1111, 0b1011'1111, 0b1011'1111}, 4, 1, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0100, 0b0000'1111, 0b1011'1111, 0b1011'1111}, 4, 1, 0xFFFDU),
			// invalid third byte
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1001'0000, 0b0000'0000, 0b1000'0000}, 4, 2, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1011'1111, 0b1111'1111, 0b1011'1111}, 4, 2, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0100, 0b1000'1111, 0b0111'1111, 0b1011'1111}, 4, 2, 0xFFFDU),
			// invalid fourth byte
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1001'0000, 0b1000'0000, 0b0000'0000}, 4, 3, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1011'1111, 0b1011'1111, 0b1111'1111}, 4, 3, 0xFFFDU),
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0100, 0b1000'1111, 0b1011'1111, 0b0111'1111}, 4, 3, 0xFFFDU),
			// missing second byte
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0, 0, 0}, 1, 1, 0xFFFDU),
			// missing third byte
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1000'1111, 0, 0}, 2, 1, 0xFFFDU),
			// missing fourth byte
			std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1000'1111, 0b1000'1111, 0}, 3, 1, 0xFFFDU)
		);

		unsigned char const* begin = std::get<0>(test).data();
		unsigned char const* const end = begin + std::get<1>(test);
		unsigned char const* const begin_control = begin + std::get<2>(test);
		std::uint32_t const codep_control = std::get<3>(test);
		CHECK(codep_control == fstlog::detail::decode_utf8_char(begin, end));
		CHECK(begin_control == begin);
	}
};

TEST_CASE("safe_utf8_to_utf16") {
	std::array<char16_t, 128> buffer;
	char16_t* const buffer_begin = buffer.data();
	
	SECTION("not_enough_space") {
		SECTION("ascii") {
			buffer.fill('!');
			std::basic_string_view input(u8"ASCII\\string/str");
			unsigned char const* const in_begin = reinterpret_cast<const unsigned char*>(input.data());
			unsigned char const* const in_end = in_begin + input.size();
			char16_t const* const buffer_end = buffer_begin + 5;
			unsigned char const* ptr = in_begin;
			auto result = fstlog::detail::safe_utf8_to_utf16(ptr, in_end, buffer_begin, buffer_end);
			CHECK(result.ec == fstlog::error_code::buff_full);
			CHECK(ptr == in_begin + 5);
			CHECK(result.ptr == buffer_begin + 5);
			CHECK(std::basic_string_view<char16_t>(buffer_begin, 5) == u"ASCII");
		}

		SECTION("multi_byte_utf8") {
			buffer.fill('!');
			std::basic_string_view input(u8"€€€");
			unsigned char const* const in_begin = reinterpret_cast<const unsigned char*>(input.data());
			unsigned char const* const in_end = in_begin + input.size();
			char16_t const* const buffer_end = buffer_begin + 1;
			unsigned char const* ptr = in_begin;
			auto result = fstlog::detail::safe_utf8_to_utf16(ptr, in_end, buffer_begin, buffer_end);
			CHECK(result.ec == fstlog::error_code::buff_full);
			CHECK(ptr == in_begin + 3);
			CHECK(result.ptr == buffer_begin + 1);
			CHECK(std::basic_string_view<char16_t>(buffer_begin, 1) == u"€");
		}
	}

	SECTION("invalid_utf8") {
		buffer.fill('!');
		std::array<unsigned char, 13> input{ 'I', 'n', 'v', 'a', 'l', 'i', 'd', ':', 128, 'c', 'h', 'a', 'r' };
		unsigned char const* const in_begin = input.data();
		unsigned char const* const in_end = in_begin + input.size();
		char16_t const* const buffer_end = buffer_begin + buffer.size();
		unsigned char const* ptr = in_begin;
		auto result = fstlog::detail::safe_utf8_to_utf16(ptr, in_end, buffer_begin, buffer_end);
		CHECK(result.ec == fstlog::error_code::input_bad);
		CHECK(ptr == in_begin + 8);
		CHECK(result.ptr == buffer_begin + 8);
		CHECK(std::basic_string_view<char16_t>(buffer_begin, 8) == u"Invalid:");
	}

	SECTION("unsafe_string") {
		buffer.fill('!');
		std::array<unsigned char, 15> input{ 'U', 'n', 's', 'a', 'f', 'e',':', 0xf0, 0x9f, 0x98, 0x80, 'c', 'h', 'a', 'r' };
		unsigned char const* const in_begin = input.data();
		unsigned char const* const in_end = in_begin + input.size();
		char16_t const* const buffer_end = buffer_begin + buffer.size();
		unsigned char const* ptr = in_begin;
		auto result = fstlog::detail::safe_utf8_to_utf16(ptr, in_end, buffer_begin, buffer_end);
		CHECK(result.ec == fstlog::error_code::input_bad);
		CHECK(ptr == in_begin + 7);
		CHECK(result.ptr == buffer_begin + 7);
		CHECK(std::basic_string_view<char16_t>(buffer_begin, 7) == u"Unsafe:");
	}

	SECTION("safe_string") {
		buffer.fill('!');
		std::basic_string_view input(u8"Safe utf8 string €€€ 编码者.");
		unsigned char const* const in_begin = reinterpret_cast<const unsigned char*>(input.data());
		unsigned char const* const in_end = in_begin + input.size();
		char16_t const* const buffer_end = buffer_begin + buffer.size();
		unsigned char const* ptr = in_begin;
		auto result = fstlog::detail::safe_utf8_to_utf16(ptr, in_end, buffer_begin, buffer_end);
		CHECK(result.ec == fstlog::error_code::none);
		CHECK(ptr == in_end);
		std::basic_string_view control{u"Safe utf8 string €€€ 编码者."};
		CHECK(result.ptr == buffer_begin + control.size());
		CHECK(std::basic_string_view<char16_t>(buffer_begin, control.size()) == control);
	}
}

TEST_CASE("safe_code_point_benchmark", "[.][benchmark]") {
	auto input = GENERATE(
		U"UTF encoding is a standard for encoding characters in the Unicode character set.",
		U"Kodowanie UTF jest standardem kodowania znaków w zestawie znaków Unicode.",
		U"Кодировка UTF является стандартом для кодирования символов в наборе символов Unicode.",
		U"UTF კოდირება არის სტანდარტი სიმბოლოების კოდირებისთვის Unicode სიმბოლოების ნაკრარში.",
		U"UTF 编码者, Unicode 字符集字符编码之准也。"
	);

	DYNAMIC_SECTION("input_") {
		auto const begin = input;
		BENCHMARK_ADVANCED("safe_code_point")(Catch::Benchmark::Chronometer meter) {
			meter.measure([begin] {
				std::size_t res = 0;
				auto pos = begin;
				while (*pos != 0) {
					res += fstlog::detail::safe_utf_code_point(*pos++);
				}
				return res;
			});
		};
	};
};

TEST_CASE("utf8_str_trim") {

	SECTION("good_data") {
		auto data = GENERATE(
			// good data
			std::make_tuple(std::basic_string_view(u8"UTF 编码者"), 0, 0, 0),
			std::make_tuple(std::basic_string_view(u8"UTF 编码者"), 1, 1, 1),
			std::make_tuple(std::basic_string_view(u8"UTF 编码者"), 5, 5, 7),
			std::make_tuple(std::basic_string_view(u8"UTF 编码者"), 7, 7, 13),
			std::make_tuple(std::basic_string_view(u8"UTF 编码者"), 100, 7, 13),
			// unsafe data
			std::make_tuple(std::basic_string_view(u8"UTF \uFFFD码者"), 5, 5, 7),
			std::make_tuple(std::basic_string_view(u8"UTF \uFFFD码者"), 7, 7, 13),
			std::make_tuple(std::basic_string_view(u8"UTF 😀码者"), 100, 7, 14)
		);

		auto input = std::get<0>(data);
		CAPTURE(reinterpret_cast<const char*>(input.data()));
		std::size_t wanted_char_num = std::get<1>(data);
		std::size_t control_char_num = std::get<2>(data);
		std::size_t control_byte_num = std::get<3>(data);
		std::size_t result_bytes = fstlog::detail::utf8_str_trim(
			input.data(),
			input.data() + input.size(),
			wanted_char_num);
		CHECK(wanted_char_num == control_char_num);
		CHECK(result_bytes == control_byte_num);
	}

	SECTION("corrupt_data") {
		auto data = GENERATE(                                //u8"UTF 😀 码 者"
			std::make_tuple(std::array<unsigned char, 16>{'U', 'T', 'F', ' ', 0x01, 0x9f, 0x98, 0x80, ' ', 0xe7, 0xa0, 0x81, ' ', 0xe8, 0x80, 0x85}, 5, 5, 5),
			std::make_tuple(std::array<unsigned char, 16>{'U', 'T', 'F', ' ', 0x01, 0x9f, 0x98, 0x80, ' ', 0xe7, 0xa0, 0x81, ' ', 0xe8, 0x80, 0x85}, 7, 7, 7),
			std::make_tuple(std::array<unsigned char, 16>{'U', 'T', 'F', ' ', 0x01, 0x9f, 0x98, 0x80, ' ', 0xe7, 0xa0, 0x81, ' ', 0xe8, 0x80, 0x85}, 100, 12, 16),
			std::make_tuple(std::array<unsigned char, 16>{'U', 'T', 'F', ' ', 0xf0, 0x01, 0x98, 0x80, ' ', 0xe7, 0xa0, 0x81, ' ', 0xe8, 0x80, 0x85}, 5, 5, 5),
			std::make_tuple(std::array<unsigned char, 16>{'U', 'T', 'F', ' ', 0xf0, 0x01, 0x98, 0x80, ' ', 0xe7, 0xa0, 0x81, ' ', 0xe8, 0x80, 0x85}, 7, 7, 7),
			std::make_tuple(std::array<unsigned char, 16>{'U', 'T', 'F', ' ', 0xf0, 0x01, 0x98, 0x80, ' ', 0xe7, 0xa0, 0x81, ' ', 0xe8, 0x80, 0x85}, 100, 12, 16),
			std::make_tuple(std::array<unsigned char, 16>{'U', 'T', 'F', ' ', 0xf0, 0x9f, 0x01, 0x80, ' ', 0xe7, 0xa0, 0x81, ' ', 0xe8, 0x80, 0x85}, 5, 5, 6),
			std::make_tuple(std::array<unsigned char, 16>{'U', 'T', 'F', ' ', 0xf0, 0x9f, 0x01, 0x80, ' ', 0xe7, 0xa0, 0x81, ' ', 0xe8, 0x80, 0x85}, 7, 7, 8),
			std::make_tuple(std::array<unsigned char, 16>{'U', 'T', 'F', ' ', 0xf0, 0x9f, 0x01, 0x80, ' ', 0xe7, 0xa0, 0x81, ' ', 0xe8, 0x80, 0x85}, 100, 11, 16),
			std::make_tuple(std::array<unsigned char, 16>{'U', 'T', 'F', ' ', 0xf0, 0x9f, 0x98, 0x01, ' ', 0xe7, 0xa0, 0x81, ' ', 0xe8, 0x80, 0x85}, 5, 5, 7),
			std::make_tuple(std::array<unsigned char, 16>{'U', 'T', 'F', ' ', 0xf0, 0x9f, 0x98, 0x01, ' ', 0xe7, 0xa0, 0x81, ' ', 0xe8, 0x80, 0x85}, 7, 7, 9),
			std::make_tuple(std::array<unsigned char, 16>{'U', 'T', 'F', ' ', 0xf0, 0x9f, 0x98, 0x01, ' ', 0xe7, 0xa0, 0x81, ' ', 0xe8, 0x80, 0x85}, 100, 10, 16)
		);

		auto input = std::get<0>(data);
		std::size_t wanted_char_num = std::get<1>(data);
		std::size_t control_char_num = std::get<2>(data);
		std::size_t control_byte_num = std::get<3>(data);
		std::size_t result_bytes = fstlog::detail::utf8_str_trim(
			input.data(),
			input.data() + input.size(),
			wanted_char_num);
		CHECK(wanted_char_num == control_char_num);
		CHECK(result_bytes == control_byte_num);
	}
}
