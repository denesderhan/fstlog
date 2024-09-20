//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma execution_character_set("utf-8")
#include <catch2/catch_all.hpp>
#include <string>
#include <cstring>

#include <detail/utf8_helper.hpp>

TEST_CASE("utf8_helper") {
	SECTION("01") {
		CHECK(fstlog::utf8_bytes('\0') == 1);
		const auto text{ u8"123 UTF-8 encoded sample ∮ E⋅da = Q,  n → ∞, ∑ f(i)"
			u8"= ∏ g(i)ði ıntəˈnæʃənəl fəˈnɛtık əsoʊsiˈeıʃn ‘single’ and"
			u8"“double” quotes• †, ‡, ‰, •, 3–4, —, −5 / +5, ™, … χαῖρε, ὦ χαῖρε, "
			u8"᾿Ελευθεριά!ტექსტების დამუშავებასა და მრავალენოვან ᚻᛖ ᚳᚹᚫᚦ ᚦᚫᛏ ᚻᛖ ᛒᚢᛞᛖ ᚩᚾ\u00a0"
			u8"ᚦᚫᛗ ᛚᚪᚾᛞᛖ⡕⠇⠙ ⡍⠜⠇⠑⠹ ⠺⠁⠎ ⠁⠎ ⠙⠑⠁⠙ ⠁⠎ ⠁ ⠙⠕⠕⠗⠤⠝⠁⠊⠇⠲ コンニチハ" };
		auto pos{ reinterpret_cast<const unsigned char*>(text) };
		const auto text_len{ std::char_traits<std::remove_pointer_t<decltype(text)>>::length(text) };
		auto end{ pos + text_len };
		std::size_t char_num{ 0 };
		while (pos < end) {
			char_num++;
			CAPTURE(char_num);
			const auto byte_num = fstlog::utf8_bytes(*pos);
			CHECK((byte_num >= 1 && byte_num <= 4));
			if (byte_num == 1) {
				CHECK(fstlog::valid_utf8(pos, pos + 1));
			}
			else if (byte_num == 2) {
				CHECK(fstlog::valid_utf8(pos, pos + 2));
				CHECK(!fstlog::valid_utf8(pos, pos + 1));
			}
			else if (byte_num == 3) {
				CHECK(fstlog::valid_utf8(pos, pos + 3));
				CHECK(!fstlog::valid_utf8(pos, pos + 1));
			}
			else if (byte_num == 4) {
				CHECK(fstlog::valid_utf8(pos, pos + 4));
				CHECK(!fstlog::valid_utf8(pos, pos + 1));
				CHECK(!fstlog::valid_utf8(pos, pos + 3));
			}
			CHECK(fstlog::valid_utf8(pos, end));
			CHECK(fstlog::printable_utf8(pos, byte_num));
			pos += byte_num;
		}
		CHECK(char_num == 300);

		std::vector<unsigned char> text2(text_len);
		memcpy(text2.data(), text, text_len);
		fstlog::sanitize_utf8_str(text2.data(), text2.data() + text_len);
		CHECK(!memcmp(text, text2.data(), text_len));
	};

	SECTION("02") {
		const auto text{ u8"\n\r\t\u0001\u000b\u001f\u007f\u0080\u009f" };
		const auto san_text{ u8"_______¡¡_" };
		auto pos{ reinterpret_cast<const unsigned char*>(text) };
		const auto text_len{ std::char_traits<std::remove_pointer_t<decltype(text)>>::length(text) + 1 };
		auto end{ pos + text_len };
		std::size_t char_num{ 0 };
		while (pos < end) {
			char_num++;
			CAPTURE(char_num);
			const auto byte_num = fstlog::utf8_bytes(*pos);
			CHECK((byte_num >= 1 && byte_num <= 4));
			if (byte_num > 1) CHECK(fstlog::utf8_bytes(*(pos + 1)) == 0);
			if (byte_num > 2) CHECK(fstlog::utf8_bytes(*(pos + 2)) == 0);
			if (byte_num > 3) CHECK(fstlog::utf8_bytes(*(pos + 3)) == 0);
			CHECK(fstlog::valid_utf8(pos, end));
			CHECK(!fstlog::printable_utf8(pos, byte_num));
			pos += byte_num;
			
		}
		CHECK(char_num == 10);

		std::vector<unsigned char> text2(text_len);
		memcpy(text2.data(), text, text_len);
		fstlog::sanitize_utf8_str(text2.data(), text2.data() + text_len);
		CHECK(!memcmp(san_text, text2.data(), text_len));
	};
}
