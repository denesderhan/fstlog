//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <vector>
#include <string_view>

#include <formatter/impl/detail/format_str_helper.hpp>
#include <fstlog/detail/error_code.hpp>

TEST_CASE("uint_fromchars_4digit") {
    auto data = GENERATE(
        std::make_tuple(std::string_view(""), 12345678, 0),
        std::make_tuple(std::string_view("a"), 12345678, 0),
        std::make_tuple(std::string_view("0"), 0, 1),
        std::make_tuple(std::string_view("0a"), 0, 1),
        std::make_tuple(std::string_view("123"), 123, 3),
        std::make_tuple(std::string_view("123a"), 123, 3),
        std::make_tuple(std::string_view("1234"), 1234, 4),
        std::make_tuple(std::string_view("1234a"), 1234, 4),
        std::make_tuple(std::string_view("0034a"), 34, 4),
        std::make_tuple(std::string_view("9999a"), 9999, 4),
        std::make_tuple(std::string_view("000034a"), 34, 6),
        std::make_tuple(std::string_view("999999a"), 9999, 6),
        std::make_tuple(std::string_view("010000a"), 1000, 6),
        std::make_tuple(std::string_view("009998a"), 9998, 6),
        std::make_tuple(std::string_view("99980a"), 9998, 5),
        std::make_tuple(std::string_view("99990a"), 9999, 5),
        std::make_tuple(std::string_view("00999000a"), 9990, 8)
    );

    auto str = std::get<0>(data);
    const auto begin = reinterpret_cast<const unsigned char*>(str.data());
    const auto end = begin + str.size();
    auto ptr = begin;
    int number = 12345678;
    fstlog::uint_fromchars_4digit(number, ptr, end);
    CHECK(number == std::get<1>(data));
    CHECK(std::get<2>(data) == ptr - begin);
}

TEST_CASE("parse_fmt_text") {
    std::vector<unsigned char> out_buff;
    
    SECTION("error_code::fmt_bad") {
        out_buff.resize(100);
        std::tuple<std::string_view, std::string_view, int> test_dat = GENERATE(
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "}" }, std::string_view{ "" }, 0 },
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ ":}" }, std::string_view{ ":" }, 1 },
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "}XXX" }, std::string_view{ "" }, 0},
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "ab:}}}" }, std::string_view{ "ab:}" }, 5},
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "{{ab:}" }, std::string_view{ "{ab:" }, 5},
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "abc}" }, std::string_view{ "abc" }, 3},
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "abc}xyz" }, std::string_view{ "abc" }, 3}
        );
        fstlog::byte_span_const input(
            reinterpret_cast<const unsigned char*>(std::get<0>(test_dat).data()),
            std::get<0>(test_dat).size());
        fstlog::byte_span output(out_buff.data(), out_buff.size());
        
        auto error = fstlog::parse_fmt_text(input, output);
        CHECK(error == fstlog::error_code::fmt_bad);
        std::string_view repl_out{ 
            reinterpret_cast<const char*>(out_buff.data()),
            static_cast<std::size_t>(out_buff.size() - output.size()) };
        CHECK(repl_out == std::get<1>(test_dat));
        CHECK(std::get<0>(test_dat).size() - input.size() == std::get<2>(test_dat));
    };

    SECTION("error_code::buff_full") {
        out_buff.resize(100);
        std::tuple<std::string_view, std::string_view, int, int> test_dat = GENERATE(
            std::tuple<std::string_view, std::string_view, int, int>{std::string_view{ "12345678901}" }, std::string_view{ "1" }, 1, 1 },
            std::tuple<std::string_view, std::string_view, int, int>{std::string_view{ "12345678901}" }, std::string_view{ "1234567890" }, 10, 10 },
            std::tuple<std::string_view, std::string_view, int, int>{std::string_view{ "}}2345{{789}}a" }, std::string_view{ "}2345{789}" }, 13, 19},
            std::tuple<std::string_view, std::string_view, int, int>{std::string_view{ "}}2345{{789}}{{" }, std::string_view{ "" }, 0, 0},
            std::tuple<std::string_view, std::string_view, int, int>{std::string_view{ "a" }, std::string_view{ "" }, 0, 0}
        );
        
        fstlog::byte_span_const input(
            reinterpret_cast<const unsigned char*>(std::get<0>(test_dat).data()),
            std::get<0>(test_dat).size());
        fstlog::byte_span output(out_buff.data(), std::get<3>(test_dat));
            
        auto error = fstlog::parse_fmt_text(input, output);
        CHECK(error == fstlog::error_code::buff_full);
        std::string_view repl_out{
            reinterpret_cast<const char*>(out_buff.data()),
            static_cast<std::size_t>(std::get<3>(test_dat) - output.size()) };
        CHECK(repl_out == std::get<1>(test_dat));
        CHECK(std::get<0>(test_dat).size() - input.size() == std::get<2>(test_dat));
    };

    SECTION("error_code::none") {
        out_buff.resize(512);
        std::tuple<std::string_view, std::string_view, int> test_dat = GENERATE(
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "{" }, std::string_view{ "" }, 0 },
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ ":{" }, std::string_view{ ":" }, 1 },
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "{XXX" }, std::string_view{ "" }, 0},
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "ab:}}{" }, std::string_view{ "ab:}" }, 5},
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "{{ab:{" }, std::string_view{ "{ab:" }, 5},
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "abc{" }, std::string_view{ "abc" }, 3},
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "abc{xyz" }, std::string_view{ "abc" }, 3},
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "" }, std::string_view{ "" }, 0 },
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "{{{{}}}}" }, std::string_view{ "{{}}" }, 8 },
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "abcd" }, std::string_view{ "abcd" }, 4 },
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "{name:}" }, std::string_view{ "" }, 0 },
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "abc{name:}" }, std::string_view{ "abc" }, 3 },
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "\x01""bc{xyz" }, std::string_view{ "\\u0001bc" }, 3},
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "\x01\x02\x03""abxyz}}{{{" }, std::string_view{ "\\u0001\\u0002\\u0003abxyz}{" }, 12},
            std::tuple<std::string_view, std::string_view, int>{std::string_view{ "abc\x01\x02:}}{{{abc" }, std::string_view{ "abc\\u0001\\u0002:}{" }, 10 }
        );
        fstlog::byte_span_const input(
            reinterpret_cast<const unsigned char*>(std::get<0>(test_dat).data()),
            std::get<0>(test_dat).size());
        fstlog::byte_span output(out_buff.data(), out_buff.size());
        
        auto error = fstlog::parse_fmt_text(input, output);
        CHECK(error == fstlog::error_code::none);
        std::string_view repl_out{ 
            reinterpret_cast<const char*>(out_buff.data()),
            static_cast<std::size_t>(out_buff.size() - output.size()) };
        CHECK(repl_out == std::get<1>(test_dat));
        CHECK(std::get<0>(test_dat).size() - input.size() == std::get<2>(test_dat));
    };
}

TEST_CASE("parse_fmt_repl_field") {
        
    SECTION("error_code::fmt_bad") {
        std::tuple<std::string_view, std::string_view, std::string_view, int> test_dat = GENERATE(
            std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abcd{xyz" }, std::string_view{ "" }, std::string_view{ "" }, 5 },
            std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abcd{{xyz" }, std::string_view{ "" }, std::string_view{ "" }, 5 },
            std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abcd:{xyz" }, std::string_view{ "" }, std::string_view{ "" }, 6 },
            std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{ab:d{xyz" }, std::string_view{ "" }, std::string_view{ "" }, 5 },
            std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abcd" }, std::string_view{ "" }, std::string_view{ "" }, 5 },
            std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abcd:xyz" }, std::string_view{ "" }, std::string_view{ "" }, 9 }
        );
        fstlog::byte_span_const input(
            reinterpret_cast<const unsigned char*>(std::get<0>(test_dat).data()),
            std::get<0>(test_dat).size());
            
        fstlog::byte_span_const field_name;
        fstlog::byte_span_const format_spec;
        auto error = fstlog::parse_fmt_repl_field(input, field_name, format_spec);
        std::string_view name{
            reinterpret_cast<const char*>(field_name.data_bytes()),
            static_cast<std::size_t>(field_name.size_bytes()) };
        std::string_view spec{
            reinterpret_cast<const char*>(format_spec.data_bytes()),
            static_cast<std::size_t>(format_spec.size_bytes()) };
        CHECK(error == fstlog::error_code::fmt_bad);
        CHECK(name == std::get<1>(test_dat));
        CHECK(spec == std::get<2>(test_dat));
        CHECK(std::get<0>(test_dat).size() - input.size() == std::get<3>(test_dat));
    };

    SECTION("error_code::none") {
        std::tuple<std::string_view, std::string_view, std::string_view, int> test_dat = GENERATE(
            std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abcd}{xyz" }, std::string_view{ "abcd" }, std::string_view{ "" }, 6 },
            std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abcd:}{xyz" }, std::string_view{ "abcd" }, std::string_view{ "" }, 7 },
            std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{ab:d}{xyz" }, std::string_view{ "ab" }, std::string_view{ "d" }, 6 },
            std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abc}d" }, std::string_view{ "abc" }, std::string_view{ "" }, 5 },
            std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abcd:xyz}" }, std::string_view{ "abcd" }, std::string_view{ "xyz" }, 10 },
            std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{:}" }, std::string_view{ "" }, std::string_view{ "" }, 3 },
            std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{}" }, std::string_view{ "" }, std::string_view{ "" }, 2 },
            std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{::}}" }, std::string_view{ "" }, std::string_view{ ":" }, 4 },
            std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abcd:xy:z}" }, std::string_view{ "abcd" }, std::string_view{ "xy:z" }, 11 }
        );
        fstlog::byte_span_const input(
            reinterpret_cast<const unsigned char*>(std::get<0>(test_dat).data()),
            std::get<0>(test_dat).size());
            
        fstlog::byte_span_const field_name;
        fstlog::byte_span_const format_spec;
        auto error = fstlog::parse_fmt_repl_field(input, field_name, format_spec);
        std::string_view name{
            reinterpret_cast<const char*>(field_name.data_bytes()),
            static_cast<std::size_t>(field_name.size_bytes()) };
        std::string_view spec{
            reinterpret_cast<const char*>(format_spec.data_bytes()),
            static_cast<std::size_t>(format_spec.size_bytes()) };
        CHECK(error == fstlog::error_code::none);
        CHECK(name == std::get<1>(test_dat));
        CHECK(spec == std::get<2>(test_dat));
        CHECK(std::get<0>(test_dat).size() - input.size() == std::get<3>(test_dat));
    };
}


TEST_CASE("skip_fill_align") {
    std::pair<std::string_view, int> test_dat = GENERATE(
        std::pair<std::string_view, int>{std::string_view{ "" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "abcd" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "123" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ ".123" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "#" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "<1234" }, 1 },
        std::pair<std::string_view, int>{std::string_view{ ">1234" }, 1 },
        std::pair<std::string_view, int>{std::string_view{ "^1234" }, 1 },
        std::pair<std::string_view, int>{std::string_view{ ">>1234" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ "<<1234" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ "^^1234" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ "a>1234" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ " >1234" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ ":>" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ ">>>>" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ "\xc2\xa9>>>1" }, 3 },
        std::pair<std::string_view, int>{std::string_view{ "\xe0\xbc\x80>>>2" }, 0 }, // non whitelisted char
        std::pair<std::string_view, int>{std::string_view{ "😉>>>3" }, 5 }, 
        std::pair<std::string_view, int>{std::string_view{ "\xf8>>>4" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "\xf8\x01>>>5" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "\xf8\x01\x02>>>6" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "\xf8\x01\x02\x03>>>7" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "\xf8\x01\x02\x03\x01>>>8" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "\xf8\x01\x02\x03\x01""a>>>9" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "a\xf8\x02\x03\x01>>>10" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "a<\xf8\x02\x03\x01>>>11" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ "\xf8<\x01\x02\x03\x01>>>12" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "a\xf8<\x01\x02\x03\x01>>>13" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "\xc2\x85>200" }, 0 }
    );
    auto str = std::get<0>(test_dat);
    CAPTURE(str);
    auto in_beg = reinterpret_cast<const unsigned char*>(str.data());
    auto in_end = in_beg + str.size();

    auto pos = fstlog::skip_fill_align(in_beg, in_end);
    CHECK(pos == in_beg + std::get<1>(test_dat));
}


TEST_CASE("skip_sign_alt_0") {
    std::pair<std::string_view, int> test_dat = GENERATE(
        std::pair<std::string_view, int>{std::string_view{ "" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "abcd" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "123" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ ".123" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "<1234" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ ">1234" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "^1234" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ ":>" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "\xf8\x01\x02>>>" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ " " }, 1 },
        std::pair<std::string_view, int>{std::string_view{ "-" }, 1 },
        std::pair<std::string_view, int>{std::string_view{ "+" }, 1 },
        std::pair<std::string_view, int>{std::string_view{ "#" }, 1 },
        std::pair<std::string_view, int>{std::string_view{ "0" }, 1 },
        std::pair<std::string_view, int>{std::string_view{ " #" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ "+#" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ "-#" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ " 0" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ "+0" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ "-0" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ "#0" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ " #0" }, 3 },
        std::pair<std::string_view, int>{std::string_view{ "+#0" }, 3 },
        std::pair<std::string_view, int>{std::string_view{ "-#0" }, 3 },
        std::pair<std::string_view, int>{std::string_view{ " abc" }, 1 },
        std::pair<std::string_view, int>{std::string_view{ "-abc" }, 1 },
        std::pair<std::string_view, int>{std::string_view{ "+abc" }, 1 },
        std::pair<std::string_view, int>{std::string_view{ "#abc" }, 1 },
        std::pair<std::string_view, int>{std::string_view{ "0abc" }, 1 },
        std::pair<std::string_view, int>{std::string_view{ " #123" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ "+#123" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ "-#.123" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ " 0.123" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ "+0ddd" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ "-0LL" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ "#0  " }, 2 },
        std::pair<std::string_view, int>{std::string_view{ " #0-#" }, 3 },
        std::pair<std::string_view, int>{std::string_view{ "+#0#+" }, 3 },
        std::pair<std::string_view, int>{std::string_view{ "-#000" }, 3 },
        std::pair<std::string_view, int>{std::string_view{ "  ##" }, 1 },
        std::pair<std::string_view, int>{std::string_view{ "-0#" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ "#-00" }, 1 },
        std::pair<std::string_view, int>{std::string_view{ "#0-" }, 2 },
        std::pair<std::string_view, int>{std::string_view{ "000" }, 1 },
        std::pair<std::string_view, int>{std::string_view{ "###" }, 1 },
        std::pair<std::string_view, int>{std::string_view{ "+- " }, 1 },
        std::pair<std::string_view, int>{std::string_view{ "0-#" }, 1 }

    );
    auto in_beg = reinterpret_cast<const unsigned char*>(std::get<0>(test_dat).data());
    auto in_end = in_beg + std::get<0>(test_dat).size();
    auto pos = fstlog::skip_sign_alt_0(in_beg, in_end);
    std::string_view test_str = std::get<0>(test_dat);
    INFO("The string was: " << test_str);
    CHECK(pos == in_beg + std::get<1>(test_dat));
}

TEST_CASE("skip_numbers") {
    std::pair<std::string_view, int> test_dat = GENERATE(
        std::pair<std::string_view, int>{std::string_view{ "" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "abcd" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ ".123" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "<1234" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ ">1234" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "^1234" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ ":>" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "\xf8\x01\x02>>>" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ " " }, 0 },
        std::pair<std::string_view, int>{std::string_view{ " 0.123" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "+0ddd" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "-0LL" }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "#0  " }, 0 },
        std::pair<std::string_view, int>{std::string_view{ "0-#" }, 1 },
        std::pair<std::string_view, int>{std::string_view{ "000123" }, 6 },
        std::pair<std::string_view, int>{std::string_view{ "000123abc" }, 6 },
        std::pair<std::string_view, int>{std::string_view{ "000123 abc" }, 6 },
        std::pair<std::string_view, int>{std::string_view{ "000123\xf8" }, 6 },
        std::pair<std::string_view, int>{std::string_view{ "0123456789abc" }, 10 },
        std::pair<std::string_view, int>{std::string_view{ "0123456789abc" }, 10 },
        std::pair<std::string_view, int>{std::string_view{ "012/" }, 3 },
        std::pair<std::string_view, int>{std::string_view{ "012:" }, 3 }
    );
    auto in_beg = reinterpret_cast<const unsigned char*>(std::get<0>(test_dat).data());
    auto in_end = in_beg + std::get<0>(test_dat).size();
    auto pos = fstlog::skip_numbers(in_beg, in_end);
    std::string_view test_str = std::get<0>(test_dat);
    INFO("The string was: " << test_str);
    CHECK(pos == in_beg + std::get<1>(test_dat));
}

TEST_CASE("get_zone") {
    std::tuple<std::string_view, fstlog::tz_format, int> test_dat = GENERATE(
        std::tuple<std::string_view, fstlog::tz_format, int>{std::string_view{ "" }, fstlog::tz_format::Local, 0 },
        std::tuple<std::string_view, fstlog::tz_format, int>{std::string_view{ "L" }, fstlog::tz_format::Local, 1 },
        std::tuple<std::string_view, fstlog::tz_format, int>{std::string_view{ "abcd" }, fstlog::tz_format::Local, 0 },
        std::tuple<std::string_view, fstlog::tz_format, int>{std::string_view{ ".123" }, fstlog::tz_format::Local, 0 },
        std::tuple<std::string_view, fstlog::tz_format, int>{std::string_view{ "<1234" }, fstlog::tz_format::Local, 0 },
        std::tuple<std::string_view, fstlog::tz_format, int>{std::string_view{ ">1234" }, fstlog::tz_format::Local, 0 },
        std::tuple<std::string_view, fstlog::tz_format, int>{std::string_view{ "^1234" }, fstlog::tz_format::Local, 0 },
        std::tuple<std::string_view, fstlog::tz_format, int>{std::string_view{ ":>" }, fstlog::tz_format::Local, 0 },
        std::tuple<std::string_view, fstlog::tz_format, int>{std::string_view{ "\xf8\x01\x02>>>" }, fstlog::tz_format::Local, 0 },
        std::tuple<std::string_view, fstlog::tz_format, int>{std::string_view{ "LUL" }, fstlog::tz_format::Local, 1 },
        std::tuple<std::string_view, fstlog::tz_format, int>{std::string_view{ "UUL" }, fstlog::tz_format::UTC, 1 },
        std::tuple<std::string_view, fstlog::tz_format, int>{std::string_view{ "ULL" }, fstlog::tz_format::UTC, 1},
        std::tuple<std::string_view, fstlog::tz_format, int>{std::string_view{ "U" }, fstlog::tz_format::UTC, 1 },
        std::tuple<std::string_view, fstlog::tz_format, int>{std::string_view{ "U\xf8" }, fstlog::tz_format::UTC, 1 }
    );
    fstlog::byte_span_const input{
        reinterpret_cast<const unsigned char*>(std::get<0>(test_dat).data()),
        std::get<0>(test_dat).size()};
    auto zone = fstlog::get_zone(input);
    CHECK(zone == std::get<1>(test_dat));
    CHECK(std::get<0>(test_dat).size() - input.size() == std::get<2>(test_dat));
}

TEST_CASE("get_width") {
    std::tuple<std::string_view, int, int> test_dat = GENERATE(
        std::tuple<std::string_view, int, int> {std::string_view{ "" }, 0, 0 },
        std::tuple<std::string_view, int, int> {std::string_view{ " 0123" }, 0, 0 },
        std::tuple<std::string_view, int, int> {std::string_view{ ".01234" }, 0, 0 },
        std::tuple<std::string_view, int, int> {std::string_view{ "#012" }, 0, 0 },
        std::tuple<std::string_view, int, int> {std::string_view{ "\xf8""012" }, 0, 0 },
        std::tuple<std::string_view, int, int> {std::string_view{ "\xc2\xa9""012" }, 0, 0 },
        std::tuple<std::string_view, int, int> {std::string_view{ "123 acc" }, 123, 3 },
        std::tuple<std::string_view, int, int> {std::string_view{ "00001234abcd" }, 1234, 8 },
        std::tuple<std::string_view, int, int> {std::string_view{ "000012340abcd" }, 1234, 9 },
        std::tuple<std::string_view, int, int> {std::string_view{ "00000abcd" }, 0, 5 },
        std::tuple<std::string_view, int, int> {std::string_view{ "9999abcd" }, 9999, 4 },
        std::tuple<std::string_view, int, int> {std::string_view{ "09999abcd" }, 9999, 5 },
        std::tuple<std::string_view, int, int> {std::string_view{ "1abcd" }, 1, 1 },
        std::tuple<std::string_view, int, int> {std::string_view{ "999" }, 999, 3 },
        std::tuple<std::string_view, int, int> {std::string_view{ "00010000" }, 1000, 8 },
        std::tuple<std::string_view, int, int> {std::string_view{ "10  " }, 10, 2 }
    );
    auto in_beg = reinterpret_cast<const unsigned char*>(std::get<0>(test_dat).data());
    auto in_end = in_beg + std::get<0>(test_dat).size();
    auto pos = in_beg;
    int width = fstlog::get_width(pos, in_end);
    std::string_view test_str = std::get<0>(test_dat);
    INFO("The string was: " << test_str);
    CHECK(width == std::get<1>(test_dat));
    CHECK(pos == in_beg + std::get<2>(test_dat));
}

TEST_CASE("get_precision") {
    std::tuple<std::string_view, int, int, int> test_dat = GENERATE(
        std::tuple<std::string_view, int, int, int> {std::string_view{ "" }, 0xffff, 0xffff, 0 },
        std::tuple<std::string_view, int, int, int> {std::string_view{ " 0123" }, 0, 0, 0 },
        std::tuple<std::string_view, int, int, int> {std::string_view{ "#012" }, 1, 1, 0 },
        std::tuple<std::string_view, int, int, int> {std::string_view{ "\xf8""012" }, 0xffff, 0xffff, 0 },
        std::tuple<std::string_view, int, int, int> {std::string_view{ "\xc2\xa9""012" }, 0xffff, 0xffff, 0 },
        std::tuple<std::string_view, int, int, int> {std::string_view{ "123 acc" }, 0xffff, 0xffff, 0 },
        std::tuple<std::string_view, int, int, int> {std::string_view{ "00001234abcd" }, 0xffff, 0xffff, 0 },
        std::tuple<std::string_view, int, int, int> {std::string_view{ "9999abcd" }, 0xffff, 0xffff, 0 },
        std::tuple<std::string_view, int, int, int> {std::string_view{ ".123 acc" }, 0xffff, 123, 4 },
        std::tuple<std::string_view, int, int, int> {std::string_view{ ".00001234abcd" }, 0xffff, 1234, 9 },
        std::tuple<std::string_view, int, int, int> {std::string_view{ ".000012340abcd" }, 0xffff, 1234, 10 },
        std::tuple<std::string_view, int, int, int> {std::string_view{ ".00000abcd" }, 0xffff, 0, 6 },
        std::tuple<std::string_view, int, int, int> {std::string_view{ ".9999abcd" }, 0xffff, 9999, 5 },
        std::tuple<std::string_view, int, int, int> {std::string_view{ ".09999abcd" }, 0xffff, 9999, 6 },
        std::tuple<std::string_view, int, int, int> {std::string_view{ ".1abcd" }, 0xffff, 1, 2 },
        std::tuple<std::string_view, int, int, int> {std::string_view{ ".999" }, 0xffff, 999, 4 },
        std::tuple<std::string_view, int, int, int> {std::string_view{ ".00010000" }, 0xffff, 1000, 9 },
        std::tuple<std::string_view, int, int, int> {std::string_view{ ".10  " }, 0xffff, 10, 3 }
    );
    auto in_beg = reinterpret_cast<const unsigned char*>(std::get<0>(test_dat).data());
    auto in_end = in_beg + std::get<0>(test_dat).size();
    auto pos = in_beg;
    int precision = std::get<1>(test_dat);
    fstlog::get_precision(precision, pos, in_end);
    std::string_view test_str = std::get<0>(test_dat);
    INFO("The string was: " << test_str);
    CHECK(precision == std::get<2>(test_dat));
    CHECK(pos == in_beg + std::get<3>(test_dat));
}

TEST_CASE("time_format") {
    auto test_dat = GENERATE(
        std::make_tuple(std::string_view{ "x<234.123abcdefgh" }, std::string_view{ "x<234" }, std::string_view{ ".123abcdefgh" }, fstlog::error_code::none),
        std::make_tuple(std::string_view{ "234.123abcdefgh" }, std::string_view{ "234" }, std::string_view{ ".123abcdefgh" }, fstlog::error_code::none),
        std::make_tuple(std::string_view{ "<.123abcdefgh" }, std::string_view{ "<" }, std::string_view{ ".123abcdefgh" }, fstlog::error_code::none),
        std::make_tuple(std::string_view{ "x<-#0234.00123abcdefgh" }, std::string_view{ "x<-#0234" }, std::string_view{ ".00123abcdefgh" }, fstlog::error_code::fmt_bad),
        std::make_tuple(std::string_view{ "x<20-#0234.00123abcdefgh" }, std::string_view{ "x<20" }, std::string_view{ "-#0234.00123abcdefgh" }, fstlog::error_code::none),
        std::make_tuple(std::string_view{ "" }, std::string_view{ "" }, std::string_view{ "" }, fstlog::error_code::none),
        std::make_tuple(std::string_view{ "1234" }, std::string_view{ "1234" }, std::string_view{ "" }, fstlog::error_code::none),
        std::make_tuple(std::string_view{ "<" }, std::string_view{ "<" }, std::string_view{ "" }, fstlog::error_code::none),
        std::make_tuple(std::string_view{ "+#0" }, std::string_view{ "+#0" }, std::string_view{ "" }, fstlog::error_code::fmt_bad)
    );
    std::string_view test_str = std::get<0>(test_dat);
    INFO("The string was: " << test_str);
    fstlog::byte_span_const format_spec(reinterpret_cast<const unsigned char*>(test_str.data()), test_str.length());
    fstlog::byte_span_const time_format;
    auto error = fstlog::time_format(format_spec, time_format);
    
    std::string_view strv_format_spec(reinterpret_cast<const char*>(format_spec.data_bytes()), format_spec.size_bytes());
    std::string_view strv_time_format(reinterpret_cast<const char*>(time_format.data_bytes()), time_format.size_bytes());

    CHECK(error == std::get<3>(test_dat));
    CHECK(strv_format_spec == std::get<1>(test_dat));
    CHECK(strv_time_format == std::get<2>(test_dat));

}

TEST_CASE("get_repl_field_[id/name]") {
    for (int ind = 0; ind < fstlog::ut_cast(fstlog::logfield_last); ind++) {
        fstlog::logfield field = fstlog::logfield(ind);
        auto name = fstlog::get_repl_field_name(field);
        fstlog::byte_span_const temp(reinterpret_cast<const unsigned char*>(name.data()), name.length());
        CHECK(field == fstlog::get_repl_field_id(temp));
    }
    CHECK(fstlog::logfield_last == fstlog::logfield::Args);
    CHECK(fstlog::get_repl_field_name(fstlog::logfield::Args) == "invalid");
}

TEST_CASE("valid_format_spec") {
    auto test_dat = GENERATE(
        std::make_tuple(std::string_view{ "<" }, true),
        std::make_tuple(std::string_view{ "x<" }, true),
        std::make_tuple(std::string_view{ "{<" }, false),
        std::make_tuple(std::string_view{ "}<" }, false),
        std::make_tuple(std::string_view{ "\xf8<" }, false),
        std::make_tuple(std::string_view{ "\xc2\xa9^" }, true),
        std::make_tuple(std::string_view{ "\xe0\xbc\x80<" }, false), // non whitelisted char
        std::make_tuple(std::string_view{ "😉>" }, true),  
        std::make_tuple(std::string_view{ "\xc2\x85<" }, false),
        std::make_tuple(std::string_view{ "*> #020.34Ld" }, true),
        std::make_tuple(std::string_view{ "*> #020.34L" }, true),
        std::make_tuple(std::string_view{ "*> #020.34d" }, true),
        std::make_tuple(std::string_view{ "*> #020Ld" }, true),
        std::make_tuple(std::string_view{ "*> #0.34Ld" }, true),
        std::make_tuple(std::string_view{ "*> #20.34Ld" }, true),
        std::make_tuple(std::string_view{ "*> 020.34Ld" }, true),
        std::make_tuple(std::string_view{ "*>#020.34Ld" }, true),
        std::make_tuple(std::string_view{ "*>-20.34Ld" }, true),
        std::make_tuple(std::string_view{ "*>#20.34Ld" }, true),
        std::make_tuple(std::string_view{ "#020.34d" }, true),
        std::make_tuple(std::string_view{ "*>." }, false),
        std::make_tuple(std::string_view{ "0020.34Ld" }, false),
        std::make_tuple(std::string_view{ "*> #020.34j" }, false),
        std::make_tuple(std::string_view{ "j" }, false),
        std::make_tuple(std::string_view{ "20000.34" }, false),
        std::make_tuple(std::string_view{ ".34" }, true),
        std::make_tuple(std::string_view{ ".34444" }, false),
        std::make_tuple(std::string_view{ "020" }, true),
        std::make_tuple(std::string_view{ "0020" }, false),
        std::make_tuple(std::string_view{ "020111" }, false),
        std::make_tuple(std::string_view{ "\xc2\xa9^ 9999.9999A" }, true),
        std::make_tuple(std::string_view{ "\xc2\xa9<+9999.9999a" }, true),
        std::make_tuple(std::string_view{ "\xc2\xa9>-9999.9999b" }, true),
        std::make_tuple(std::string_view{ "\xc2\xa9^#9999.9999B" }, true),
        std::make_tuple(std::string_view{ "\xc2\xa9< 9999.9999x" }, true),
        std::make_tuple(std::string_view{ "<^9999.9999X" }, true),
        std::make_tuple(std::string_view{ ">^.9999o" }, true),
        std::make_tuple(std::string_view{ "9999p" }, true)
    );
    CAPTURE(std::get<0>(test_dat).data());
    fstlog::byte_span_const fmt_spec(
        reinterpret_cast<const unsigned char*>(std::get<0>(test_dat).data()),
        std::get<0>(test_dat).size());
    auto is_valid = fstlog::valid_format_spec(fmt_spec);
    CHECK(is_valid == std::get<1>(test_dat));
}

TEST_CASE("valid_fmt_type_spec") {
    for (int t = 0; t < 256; t++) {
        unsigned char type_spec = static_cast<unsigned char>(t);
        CAPTURE(int(type_spec));
        switch (type_spec) {
        case 'a': case 'A': case 'b': case 'B': case 'c': case 'd':
        case 'e': case 'E': case 'f': case 'F': case 'g': case 'G':
        case 'o': case 'p': case 's': case 'x': case 'X':
            CHECK(fstlog::valid_fmt_type_spec(type_spec) == true);
            break;
        default:
            CHECK(fstlog::valid_fmt_type_spec(type_spec) == false);
            break;
        }
    }
}

TEST_CASE("skip_valid_fmt_number") {
    auto test_dat = GENERATE(
        std::make_tuple(std::string_view{ "" }, true, 0),
        std::make_tuple(std::string_view{ "abcd" }, true, 0),
        std::make_tuple(std::string_view{ "1abc" }, true, 1),
        std::make_tuple(std::string_view{ "12a3abc" }, true, 2),
        std::make_tuple(std::string_view{ "123abc" }, true, 3),
        std::make_tuple(std::string_view{ "1000abc" }, true, 4),
        std::make_tuple(std::string_view{ "9123abc" }, true, 4),
        std::make_tuple(std::string_view{ "0abc" }, false, 1),
        std::make_tuple(std::string_view{ "00abc" }, false, 1),
        std::make_tuple(std::string_view{ "0123abc" }, false, 1),
        std::make_tuple(std::string_view{ "80000dsf" }, false, 5),
        std::make_tuple(std::string_view{ "80000234dsf" }, false, 5),
        std::make_tuple(std::string_view{ "1" }, true, 1),
        std::make_tuple(std::string_view{ "12" }, true, 2),
        std::make_tuple(std::string_view{ "123" }, true, 3),
        std::make_tuple(std::string_view{ "1000" }, true, 4),
        std::make_tuple(std::string_view{ "9123" }, true, 4)
    );
    const auto in_beg = reinterpret_cast<const unsigned char*>(std::get<0>(test_dat).data());
    auto pos = in_beg;
    auto in_end = in_beg + std::get<0>(test_dat).size();

    CHECK(fstlog::skip_valid_fmt_number(pos, in_end) == std::get<1>(test_dat));
    CHECK(pos - in_beg == std::get<2>(test_dat));
}
