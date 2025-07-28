//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <array>
#include <cstring>

#include <formatter/impl/detail/shift_fill.hpp>

TEST_CASE("fill_with_pattern") {
	std::array<unsigned char, 30> buffer{ 0 };
	std::array<unsigned char, 4> pattern{ 1, 2, 3, 4 };

	SECTION("zero size dest") {
		std::array<unsigned char, 30> control{ 0 };
		buffer.fill(0);
		fstlog::detail::fill_with_pattern({ buffer.data(), 0 }, pattern, 1);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		buffer.fill(0);
		fstlog::detail::fill_with_pattern({ buffer.data(), 0 }, pattern, 2);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		buffer.fill(0);
		fstlog::detail::fill_with_pattern({ buffer.data(), 0 }, pattern, 3);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		buffer.fill(0);
		fstlog::detail::fill_with_pattern({ buffer.data(), 0 }, pattern, 4);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	};

	SECTION("one size dest") {
		std::array<unsigned char, 30> control{ 0 };
		buffer.fill(0);
		fstlog::detail::fill_with_pattern({buffer.data(), 1}, pattern, 1);
		control[0] = 1;
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	};

	SECTION("two size dest") {
		std::array<unsigned char, 30> control{ 0 };
		buffer.fill(0);
		fstlog::detail::fill_with_pattern({buffer.data(), 2}, pattern, 1);
		control[0] = 1;
		control[1] = 1;
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));

		buffer.fill(0);
		fstlog::detail::fill_with_pattern({buffer.data(), 2}, pattern, 2);
		control[0] = 1;
		control[1] = 2;
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		
		buffer.fill(0);
		fstlog::detail::fill_with_pattern({buffer.data(), 10}, pattern, 2);
		control.fill(0);
		control = { 1, 2, 1, 2, 1, 2, 1, 2, 1, 2 };
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	};
}

TEST_CASE("shift_fill_single_byte_pattern") {
	std::array<unsigned char, 30> buffer{ "abcdefgh" };
	fstlog::format_setting_txt format{};
	format.fill_char[0] = 'x';

	SECTION("zero shift") {
		std::array<unsigned char, 30> control{ "abcdefgh" };
		format.width = 5;
		format.align = '^';
		fstlog::detail::shift_fill(
			{ buffer.data(), 20 },
			{ 8, 5 }, 
			format);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("empty_string") {
		std::array<unsigned char, 30> control{ 0 };
		format.width = 6;
		format.align = '<';
		const auto control_txt = "xxxxxx";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer.fill(0);
		fstlog::detail::shift_fill(
			{ buffer.data(), 30 },
			{ 0, 0 },
			format);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));

		format.align = '>';
		buffer.fill(0);
		fstlog::detail::shift_fill(
			{ buffer.data(), 30 },
			{ 0, 0 },
			format);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));

		format.align = '^';
		buffer.fill(0);
		fstlog::detail::shift_fill(
			{ buffer.data(), 30 },
			{ 0, 0 },
			format);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("1 char shift_fill left") {
		std::array<unsigned char, 30> control{ 0 };
		format.width = 6;
		format.align = '<';
		const auto control_txt = "abcdefghx";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::detail::shift_fill(
			{ buffer.data(), 20 },
			{8, 5}, 
			format);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("1 char shift_fill right") {
		std::array<unsigned char, 30> control{ 0 };
		const auto control_txt = "xabcdefgh";
		format.width = 6;
		format.align = '>';
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::detail::shift_fill(
			{buffer.data(), 20},
			{ 8, 5 },
			format);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("1 char shift_fill center") {
		std::array<unsigned char, 30> control{ 0 };
		const auto control_txt = "abcdefghx";
		format.width = 6;
		format.align = '^';
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::detail::shift_fill(
			{buffer.data(), 20},
			{ 8, 5 }, 
			format);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("3 char shift_fill left") {
		std::array<unsigned char, 30> control{ 0 };
		const auto control_txt = "abcdefghxxx";
		format.width = 8;
		format.align = '<';
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::detail::shift_fill(
			{buffer.data(), 20},
			{ 8, 5 }, 
			format);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("3 char shift_fill right") {
		std::array<unsigned char, 30> control{ 0 };
		const auto control_txt = "xxxabcdefgh";
		format.width = 8;
		format.align = '>';
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::detail::shift_fill(
			{buffer.data(), 20},
			{ 8, 5 }, 
			format);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("3 char shift_fill center") {
		std::array<unsigned char, 30> control{ 0 };
		format.width = 8;
		format.align = '^';
		const auto control_txt = "xabcdefghxx";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::detail::shift_fill(
			{buffer.data(), 20},
			{ 8, 5 }, 
			format);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}
}

TEST_CASE("shift_fill_multi_byte_pattern") {
	std::array<unsigned char, 30> buffer{ "abcdefgh" };
	fstlog::format_setting_txt format{};
	format.fill_char[0] = 0xc3;
	format.fill_char[1] = 0x81;

	SECTION("zero shift") {
		std::array<unsigned char, 30> control{ "abcdefgh" };
		format.width = 5;
		format.align = '^';
		fstlog::detail::shift_fill(
			{buffer.data(), 20}, 
			{ 8, 5 }, 
			format);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("1 char shift_fill left") {
		std::array<unsigned char, 30> control{ 0 };
		const auto control_txt = "abcdefgh\xc3\x81";
		format.width = 6;
		format.align = '<';
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::detail::shift_fill(
			{buffer.data(), 20},
			{ 8, 5 }, 
			format);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("1 char shift_fill right") {
		std::array<unsigned char, 30> control{ 0 };
		format.width = 6;
		format.align = '>';
		const auto control_txt = "\xc3\x81""abcdefgh";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::detail::shift_fill(
			{buffer.data(), 20},
			{ 8, 5 },
			format);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("1 char shift_fill center") {
		std::array<unsigned char, 30> control{ 0 };
		format.width = 6;
		format.align = '^';
		const auto control_txt = "abcdefgh\xc3\x81";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::detail::shift_fill(
			{buffer.data(), 20},
			{ 8, 5 }, format);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("3 char shift_fill left") {
		std::array<unsigned char, 30> control{ 0 };
		format.width = 8;
		format.align = '<';
		const auto control_txt = "abcdefgh\xc3\x81\xc3\x81\xc3\x81";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::detail::shift_fill(
			{buffer.data(), 20},
			{ 8, 5 }, 
			format);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("3 char shift_fill right") {
		std::array<unsigned char, 30> control{ 0 };
		format.width = 8;
		format.align = '>';
		const auto control_txt = "\xc3\x81\xc3\x81\xc3\x81""abcdefgh";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::detail::shift_fill(
			{buffer.data(), 20},
			{ 8, 5 }, 
			format);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("3 char shift_fill center") {
		std::array<unsigned char, 30> control{ 0 };
		format.width = 8;
		format.align = '^';
		const auto control_txt = "\xc3\x81""abcdefgh\xc3\x81\xc3\x81";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::detail::shift_fill(
			{buffer.data(), 20},
			{ 8, 5 }, 
			format);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}
}

