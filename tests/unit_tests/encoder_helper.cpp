//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <algorithm>
#include <charconv>
#include <array>

#include <formatter/impl/detail/encoder_helper.hpp>
#include <fstlog/detail/ut_cast.hpp>

TEST_CASE("sanitize_int_type_char") {
    for (int i = 0; i < 256; i++) {
        unsigned char type_char = static_cast<unsigned char>(i);
        std::array<unsigned char, 6> valid_chars{'b', 'B', 'd', 'o', 'x', 'X'};
        if (std::find(valid_chars.begin(), valid_chars.end(), type_char) != valid_chars.end()) {
            CHECK(fstlog::detail::sanitize_int_type_char(type_char) == type_char);
        }
        else {
            CHECK(fstlog::detail::sanitize_int_type_char(type_char) == 0);
        }
    }
}

TEST_CASE("sanitize_float_type_char") {
    for (int i = 0; i < 256; i++) {
        unsigned char type_char = static_cast<unsigned char>(i);
        std::array<unsigned char, 8> valid_chars{ 'a', 'A', 'e', 'E', 'f', 'F', 'g', 'G' };
        if (std::find(valid_chars.begin(), valid_chars.end(), type_char) != valid_chars.end()) {
            CHECK(fstlog::detail::sanitize_float_type_char(type_char) == type_char);
        }
        else {
            CHECK(fstlog::detail::sanitize_float_type_char(type_char) == 0);
        }
    }
}

TEST_CASE("get_int_base") {
    CHECK(fstlog::detail::get_int_base(0) == 10);
    CHECK(fstlog::detail::get_int_base('b') == 2);
    CHECK(fstlog::detail::get_int_base('B') == 2);
    CHECK(fstlog::detail::get_int_base('d') == 10);
    CHECK(fstlog::detail::get_int_base('o') == 8);
    CHECK(fstlog::detail::get_int_base('x') == 16);
    CHECK(fstlog::detail::get_int_base('X') == 16);
}

TEST_CASE("charconv_float_format") {
    CHECK(fstlog::detail::charconv_float_format(0) == fstlog::ut_cast(std::chars_format::general));
    CHECK(fstlog::detail::charconv_float_format('a') == fstlog::ut_cast(std::chars_format::hex));
    CHECK(fstlog::detail::charconv_float_format('A') == fstlog::ut_cast(std::chars_format::hex));
    CHECK(fstlog::detail::charconv_float_format('e') == fstlog::ut_cast(std::chars_format::scientific));
    CHECK(fstlog::detail::charconv_float_format('E') == fstlog::ut_cast(std::chars_format::scientific));
    CHECK(fstlog::detail::charconv_float_format('f') == fstlog::ut_cast(std::chars_format::fixed));
    CHECK(fstlog::detail::charconv_float_format('F') == fstlog::ut_cast(std::chars_format::fixed));
    CHECK(fstlog::detail::charconv_float_format('g') == fstlog::ut_cast(std::chars_format::general));
    CHECK(fstlog::detail::charconv_float_format('G') == fstlog::ut_cast(std::chars_format::general));
}

TEST_CASE("int_write_prefix") {
    std::array<unsigned char, 4> buff;
    auto data = GENERATE(
        std::make_tuple(0, 10, decltype(buff){'!', '!', '!', '!'}, 0),
        std::make_tuple('b', 2, decltype(buff){'0', 'b', '!', '!'}, 2),
        std::make_tuple('B', 2, decltype(buff){'0', 'B', '!', '!'}, 2),
        std::make_tuple('d', 10, decltype(buff){'!', '!', '!', '!'}, 0),
        std::make_tuple('o', 8, decltype(buff){'0', '!', '!', '!'}, 1),
        std::make_tuple('x', 16, decltype(buff){'0', 'x', '!', '!'}, 2),
        std::make_tuple('X', 16, decltype(buff){'0', 'X', '!', '!'}, 2)
    );
    buff.fill('!');
    auto pos = buff.data();

    fstlog::detail::int_write_prefix(std::get<1>(data), static_cast<unsigned char>(std::get<0>(data)), pos);
    CHECK(pos == buff.data() + std::get<3>(data));
    CHECK(buff == std::get<2>(data));    
}

TEST_CASE("write_sign") {
    std::array<unsigned char, 2> buff;
    auto data = GENERATE(
        std::make_tuple(' ', true, decltype(buff){'-', '!'}, 1),
        std::make_tuple(' ', false, decltype(buff){' ', '!'}, 1),
        std::make_tuple('-', true, decltype(buff){'-', '!'}, 1),
        std::make_tuple('-', false, decltype(buff){'!', '!'}, 0),
        std::make_tuple('+', true, decltype(buff){'-', '!'}, 1),
        std::make_tuple('+', false, decltype(buff){'+', '!'}, 1)
    );
    buff.fill('!');
    auto pos = buff.data();

    fstlog::detail::write_sign(static_cast<unsigned char>(std::get<0>(data)), std::get<1>(data), pos);
    CHECK(pos == buff.data() + std::get<3>(data));
    CHECK(buff == std::get<2>(data));
}

TEST_CASE("num_to_upper_case") {
    auto data = GENERATE(
        std::make_tuple(std::array<char, 10>{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9' }, 10, std::array<char, 10>{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9' }),
        std::make_tuple(std::array<char, 10>{'x', 'b', 'X', 'B', 'a', 'c', 'd', 'e', 'f', '.' }, 10, std::array<char, 10>{'X', 'B', 'X', 'B', 'A', 'C', 'D', 'E', 'F', '.' }),
        std::make_tuple(std::array<char, 10>{'A', 'B', 'C', 'D', 'E', 'F', ' ', '-', '+', 'f' }, 9, std::array<char, 10>{'A', 'B', 'C', 'D', 'E', 'F', ' ', '-', '+', 'f' })
    );
    auto buff = std::get<0>(data);
    fstlog::detail::num_to_upper_case(buff.data(), buff.data() + std::get<1>(data));
    CHECK(buff == std::get<2>(data));
}

TEST_CASE("abs_unsigned") {
    constexpr auto int_min = (std::numeric_limits<int>::min)();
    long long num = int_min;
    num = -num;

    CHECK(static_cast<long long>(fstlog::detail::abs_unsigned(int_min)) == num);
    CHECK(fstlog::detail::abs_unsigned(10) == 10U);
    CHECK(fstlog::detail::abs_unsigned((std::numeric_limits<int>::max)()) 
        == static_cast<unsigned int>((std::numeric_limits<int>::max)()));
}
