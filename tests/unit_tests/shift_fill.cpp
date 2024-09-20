//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <array>
#include <cstring>

#include <formatter/impl/detail/shift_fill.hpp>

TEST_CASE("fill_with_pattern") {
	std::array<unsigned char, 30> buffer{ 0 };
	std::array<unsigned char, 10> pattern{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	SECTION("zero size dest") {
		std::array<unsigned char, 30> control{ 0 };
		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 0, pattern.data(), 1);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 0, pattern.data(), 2);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 0, pattern.data(), 3);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 0, pattern.data(), 10);
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	};

	SECTION("one size dest") {
		std::array<unsigned char, 30> control{ 0 };
		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 1, pattern.data(), 1);
		control[0] = 1;
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 1, pattern.data() + 1, 1);
		control[0] = 2;
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 1, pattern.data() + 4, 1);
		control[0] = 5;
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 1, pattern.data() + 9, 1);
		control[0] = 10;
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	};

	SECTION("two size dest") {
		std::array<unsigned char, 30> control{ 0 };
		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 2, pattern.data(), 1);
		control[0] = 1;
		control[1] = 1;
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 2, pattern.data() + 1, 1);
		control[0] = 2;
		control[1] = 2;
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 2, pattern.data() + 4, 1);
		control[0] = 5;
		control[1] = 5;
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 2, pattern.data() + 9, 1);
		control[0] = 10;
		control[1] = 10;
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));

		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 2, pattern.data(), 2);
		control[0] = 1;
		control[1] = 2;
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 2, pattern.data() + 1, 2);
		control[0] = 2;
		control[1] = 3;
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 2, pattern.data() + 4, 2);
		control[0] = 5;
		control[1] = 6;
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 2, pattern.data() + 8, 2);
		control[0] = 9;
		control[1] = 10;
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));

		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 10, pattern.data(), 2);
		control.fill(0);
		control = { 1, 2, 1, 2, 1, 2, 1, 2, 1, 2 };
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 8, pattern.data() + 1, 4);
		control.fill(0);
		control = { 2, 3, 4, 5, 2, 3, 4, 5 };
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 10, pattern.data() + 4, 5);
		control.fill(0);
		control = { 5, 6, 7, 8, 9, 5, 6, 7, 8, 9 };
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
		buffer.fill(0);
		fstlog::fill_with_pattern(buffer.data(), 9, pattern.data() + 1, 3);
		control.fill(0);
		control = { 2, 3, 4, 2, 3, 4, 2, 3, 4, };
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	};
}

TEST_CASE("shift_fill_single_byte_pattern") {
	std::array<unsigned char, 30> buffer{ "abcdefgh" };
	const auto pattern{ "x" };

	SECTION("zero shift") {
		std::array<unsigned char, 30> control{ "abcdefgh" };
		fstlog::shift_fill(
			buffer.data(), buffer.data() + 20,
			8, 5, 5, '^',
			reinterpret_cast<const unsigned char*>(pattern));
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("1 char shift_fill left") {
		std::array<unsigned char, 30> control{ 0 };
		const auto control_txt = "abcdefghx";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::shift_fill(
			buffer.data(), buffer.data() + 20,
			8, 5, 6, '<',
			reinterpret_cast<const unsigned char*>(pattern));
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("1 char shift_fill right") {
		std::array<unsigned char, 30> control{ 0 };
		const auto control_txt = "xabcdefgh";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::shift_fill(
			buffer.data(), buffer.data() + 20,
			8, 5, 6, '>',
			reinterpret_cast<const unsigned char*>(pattern));
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("1 char shift_fill center") {
		std::array<unsigned char, 30> control{ 0 };
		const auto control_txt = "abcdefghx";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::shift_fill(
			buffer.data(), buffer.data() + 20,
			8, 5, 6, '^',
			reinterpret_cast<const unsigned char*>(pattern));
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("3 char shift_fill left") {
		std::array<unsigned char, 30> control{ 0 };
		const auto control_txt = "abcdefghxxx";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::shift_fill(
			buffer.data(), buffer.data() + 20,
			8, 5, 8, '<',
			reinterpret_cast<const unsigned char*>(pattern));
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("3 char shift_fill right") {
		std::array<unsigned char, 30> control{ 0 };
		const auto control_txt = "xxxabcdefgh";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::shift_fill(
			buffer.data(), buffer.data() + 20,
			8, 5, 8, '>',
			reinterpret_cast<const unsigned char*>(pattern));
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("3 char shift_fill center") {
		std::array<unsigned char, 30> control{ 0 };
		const auto control_txt = "xabcdefghxx";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::shift_fill(
			buffer.data(), buffer.data() + 20,
			8, 5, 8, '^',
			reinterpret_cast<const unsigned char*>(pattern));
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}
}

TEST_CASE("shift_fill_multi_byte_pattern") {
	std::array<unsigned char, 30> buffer{ "abcdefgh" };
	const auto pattern{ "\xc3\x81" };

	SECTION("zero shift") {
		std::array<unsigned char, 30> control{ "abcdefgh" };
		fstlog::shift_fill(
			buffer.data(), buffer.data() + 20, 
			8, 5, 5, '^', 
			reinterpret_cast<const unsigned char*>(pattern));
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("1 char shift_fill left") {
		std::array<unsigned char, 30> control{ 0 };
		const auto control_txt = "abcdefgh\xc3\x81";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::shift_fill(
			buffer.data(), buffer.data() + 20,
			8, 5, 6, '<',
			reinterpret_cast<const unsigned char*>(pattern));
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("1 char shift_fill right") {
		std::array<unsigned char, 30> control{ 0 };
		const auto control_txt = "\xc3\x81""abcdefgh";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::shift_fill(
			buffer.data(), buffer.data() + 20,
			8, 5, 6, '>',
			reinterpret_cast<const unsigned char*>(pattern));
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("1 char shift_fill center") {
		std::array<unsigned char, 30> control{ 0 };
		const auto control_txt = "abcdefgh\xc3\x81";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::shift_fill(
			buffer.data(), buffer.data() + 20,
			8, 5, 6, '^',
			reinterpret_cast<const unsigned char*>(pattern));
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("3 char shift_fill left") {
		std::array<unsigned char, 30> control{ 0 };
		const auto control_txt = "abcdefgh\xc3\x81\xc3\x81\xc3\x81";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::shift_fill(
			buffer.data(), buffer.data() + 20,
			8, 5, 8, '<',
			reinterpret_cast<const unsigned char*>(pattern));
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("3 char shift_fill right") {
		std::array<unsigned char, 30> control{ 0 };
		const auto control_txt = "\xc3\x81\xc3\x81\xc3\x81""abcdefgh";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::shift_fill(
			buffer.data(), buffer.data() + 20,
			8, 5, 8, '>',
			reinterpret_cast<const unsigned char*>(pattern));
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}

	SECTION("3 char shift_fill center") {
		std::array<unsigned char, 30> control{ 0 };
		const auto control_txt = "\xc3\x81""abcdefgh\xc3\x81\xc3\x81";
		memcpy(control.data(), control_txt, strlen(reinterpret_cast<const char*>(control_txt)));
		buffer = { "abcdefgh" };
		fstlog::shift_fill(
			buffer.data(), buffer.data() + 20,
			8, 5, 8, '^',
			reinterpret_cast<const unsigned char*>(pattern));
		CHECK(!memcmp(control.data(), buffer.data(), buffer.size()));
	}
}
