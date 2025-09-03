//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <array>
#include <cstdint>
#include <type_traits>

#include <detail/mixin/memory_resource_mixin.hpp>
#include <detail/mixin/error_state_mixin.hpp>
#include <formatter/impl/output_span_mixin.hpp>
#include <formatter/impl/detail/encoder_charconv_mixin.hpp>

using enc_type = fstlog::encoder_charconv_mixin<
                    fstlog::error_state_mixin<
                    fstlog::output_span_mixin<
                    fstlog::memory_resource_mixin>>>;

TEST_CASE("encoder_charconv_mixin") {
    SECTION("no_space_in_buffer") {
        enc_type encoder(fstlog::get_default_resource());
        std::array<unsigned char, 10> buffer{ '!' };
        buffer.fill('!');
        enc_type::format_type format;

        encoder.output_span_init(buffer);
        encoder.encode(std::string_view{ "This will be longer than 10 characters!" }, format);
        
        auto res = std::string_view(
            reinterpret_cast<char*>(encoder.output_begin()),
            encoder.output_ptr() - encoder.output_begin());
        CAPTURE(res);
        CHECK(encoder.has_error());
        CHECK(encoder.get_error().code() == fstlog::error_code::buff_full);
        CHECK(res == "This will ");
    };
    
    SECTION("bool") {
        enc_type encoder(fstlog::get_default_resource());
        std::array<unsigned char, 128> buffer{ '!' };
        auto extent = GENERATE(table<bool, char, bool, std::string_view>({
            std::tuple<bool, char, bool, std::string_view>{true, 's', false, "true"},
            std::tuple<bool, char, bool, std::string_view>{false, 's', true, "false"},
            std::tuple<bool, char, bool, std::string_view>{true, '\x00', false, "true"},
            std::tuple<bool, char, bool, std::string_view>{false, '\x00', true, "false"},
            std::tuple<bool, char, bool, std::string_view>{true, 'x', false, "1"},
            std::tuple<bool, char, bool, std::string_view>{false, 'x', true, "0x0"},
            std::tuple<bool, char, bool, std::string_view>{true, 'X', false, "1"},
            std::tuple<bool, char, bool, std::string_view>{false, 'X', true, "0X0"},
            std::tuple<bool, char, bool, std::string_view>{true, 'b', false, "1"},
            std::tuple<bool, char, bool, std::string_view>{false, 'b', true, "0b0"},
            std::tuple<bool, char, bool, std::string_view>{true, 'B', false, "1"},
            std::tuple<bool, char, bool, std::string_view>{false, 'B', true, "0B0"},
            std::tuple<bool, char, bool, std::string_view>{true, 'd', false, "1"},
            std::tuple<bool, char, bool, std::string_view>{false, 'd', true, "0"},
            std::tuple<bool, char, bool, std::string_view>{true, 'o', true, "01"},
            std::tuple<bool, char, bool, std::string_view>{true, 'o', false, "1"},
            std::tuple<bool, char, bool, std::string_view>{false, 'o', true, "0"},
            std::tuple<bool, char, bool, std::string_view>{false, 'o', false, "0"}
            }));
                
        auto to_encode = std::get<0>(extent);
        auto format = enc_type::format_type{};
        format.type = static_cast<unsigned char>(std::get<1>(extent));
        format.alternate = std::get<2>(extent);
        std::string_view control = std::get<3>(extent);
        
        CAPTURE(to_encode, char(format.type), format.alternate);

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

    SECTION("bool_bad_format_type") {
        enc_type encoder(fstlog::get_default_resource());
        std::array<unsigned char, 8> buffer{ '!' };
        fstlog::format_setting_txt format{};
        for (int i = 1; i < 256; i++) {
            const char* good = "sxXbBdo";
            while (*good != 0 && *good != i) good++;
            if (*good != 0) continue;
            format.type = static_cast<unsigned char>(i);
            buffer.fill('!');
            encoder.output_span_init(buffer);
            encoder.encode(true, format);
            encoder.encode(false, format);
            auto res = std::string_view(
                reinterpret_cast<char*>(encoder.output_begin()),
                encoder.output_ptr() - encoder.output_begin());
            CAPTURE(res);
            CHECK(!encoder.has_error());
            CHECK(res == "10");
        }
    };

    SECTION("void*") {
        auto extent = GENERATE(table<std::uintptr_t, char, bool, std::string_view>({
            std::tuple<std::uintptr_t, char, bool, std::string_view>{0x12345, 'p', false, "0x12345"},
            std::tuple<std::uintptr_t, char, bool, std::string_view>{0x12345, '\x00', true, "0x12345"}
            }));

        auto to_encode = reinterpret_cast<void*>(std::get<0>(extent));
        auto format = enc_type::format_type{};
        format.type = static_cast<unsigned char>(std::get<1>(extent));
        format.alternate = std::get<2>(extent);
        std::string_view control = std::get<3>(extent);

        CAPTURE(to_encode, char(format.type), format.alternate);

        enc_type encoder(fstlog::get_default_resource());
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

    SECTION("void*_bad_format_type") {
        enc_type encoder(fstlog::get_default_resource());
        std::array<unsigned char, 8> buffer{ '!' };
        fstlog::format_setting_txt format{};
        for (int i = 1; i < 256; i++) {
            if (i == 'p') continue;
            format.type = static_cast<unsigned char>(i);
            buffer.fill('!');
            encoder.output_span_init(buffer);
            encoder.encode(reinterpret_cast<void*>(std::uintptr_t{ 0x12345 }), format);
            auto res = std::string_view(
                reinterpret_cast<char*>(encoder.output_begin()),
                encoder.output_ptr() - encoder.output_begin());
            CAPTURE(res);
            CHECK(!encoder.has_error());
            CHECK(res == "0x12345");
        }
    };

    SECTION("integer") {
        auto extent = GENERATE(table<int, char,  char, bool, std::string_view>({
            std::tuple<int, char,  char, bool, std::string_view>{(std::numeric_limits<std::int32_t>::min)(), '\x00', '+', false, "-2147483648"},
            std::tuple<int, char,  char, bool, std::string_view>{0, '\x00', '+', false, "+0"},
            std::tuple<int, char,  char, bool, std::string_view>{0, '\x00', '-', false, "0"},
            std::tuple<int, char,  char, bool, std::string_view>{1234, '\x00', '+', false, "+1234"},
            std::tuple<int, char,  char, bool, std::string_view>{(std::numeric_limits<std::int32_t>::min)(), 'd', '+', false, "-2147483648"},
            std::tuple<int, char,  char, bool, std::string_view>{0, 'd', '+', false, "+0"},
            std::tuple<int, char,  char, bool, std::string_view>{0, 'd', '-', false, "0"},
            std::tuple<int, char,  char, bool, std::string_view>{1234, 'd', '+', false, "+1234"},
            std::tuple<int, char,  char, bool, std::string_view>{-15, 'x', ' ', false, "-f"},
            std::tuple<int, char,  char, bool, std::string_view>{15, 'x', ' ', true, " 0xf"},
            std::tuple<int, char,  char, bool, std::string_view>{15, 'X', '-', true, "0XF"},
            std::tuple<int, char,  char, bool, std::string_view>{15, 'X', '-', false, "F"},
            std::tuple<int, char,  char, bool, std::string_view>{-15, 'b', '+', true, "-0b1111"},
            std::tuple<int, char,  char, bool, std::string_view>{-15, 'b', '+', false, "-1111"},
            std::tuple<int, char,  char, bool, std::string_view>{-15, 'B', ' ', true, "-0B1111"},
            std::tuple<int, char,  char, bool, std::string_view>{15, 'B', ' ', false, " 1111"},
            std::tuple<int, char,  char, bool, std::string_view>{64, 'o', '-', false, "100"},
            std::tuple<int, char,  char, bool, std::string_view>{64, 'o', '+', true, "+0100"},
            std::tuple<int, char,  char, bool, std::string_view>{0, 'o', '+', true, "+0"}
            }));

        auto to_encode = std::get<0>(extent);
        auto format = enc_type::format_type{};
        format.type = static_cast<unsigned char>(std::get<1>(extent));
        format.sign = static_cast<unsigned char>(std::get<2>(extent));
        format.alternate = std::get<3>(extent);
        std::string_view control = std::get<4>(extent);

        CAPTURE(to_encode, char(format.type), char(format.sign), format.alternate);

        enc_type encoder(fstlog::get_default_resource());
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

    SECTION("integer_bad_format_type") {
        enc_type encoder(fstlog::get_default_resource());
        std::array<unsigned char, 8> buffer{ '!' };
        fstlog::format_setting_txt format{};
        for (int i = 1; i < 256; i++) {
            const char* good = "xXbBdo";
            while (*good != 0 && *good != i) good++;
            if (*good != 0) continue;
            format.type = static_cast<unsigned char>(i);
            buffer.fill('!');
            encoder.output_span_init(buffer);
            encoder.encode(99999, format);
            auto res = std::string_view(
                reinterpret_cast<char*>(encoder.output_begin()),
                encoder.output_ptr() - encoder.output_begin());
            CAPTURE(i, res);
            CHECK(!encoder.has_error());
            CHECK(res == "99999");
        }
    };

    SECTION("floating_point") {
        //                                type  sign  alternate   precision
        using tup_typ = std::tuple<float, char, char, bool, int, std::string_view>;
        auto extent = GENERATE(table<float, char, char, bool, int, std::string_view>({
            tup_typ{0.12434234233422f, '\x00', '+', false, 4, "+0.1243"},
            tup_typ{1.12378f, 'e', ' ', false, 3, " 1.124e+00"},
            tup_typ{-1.12378f, 'e', '+', false, 3, "-1.124e+00"},
            tup_typ{1.11f, 'g', '-', false, 2, "1.1"},
            tup_typ{1.11f, 'f', '-', false, 2, "1.11"},
            tup_typ{0.11f, 'f', '-', false, 2, "0.11"},
            tup_typ{0.11f, 'g', '-', false, 2, "0.11"},
            tup_typ{1.209f, 'g', '-', false, 2, "1.2"},
            tup_typ{0.209f, 'g', '-', false, 2, "0.21"},
            tup_typ{0.4567f, 'g', '-', false, 2, "0.46"},
            
            tup_typ{0.0f, 'a', '-', false, 3, "0.000p+0"},
            tup_typ{0.0f, 'A', '-', false, enc_type::format_type{}.precision, "0P+0"},
            tup_typ{0.0f, 'a', '-', true, 3, "0.000p+0"},
            //alternate not implemented
            //tup_typ{0.0f, 'A', '-', true, enc_type::format_type{}.precision, "0.P+0"},
            //tup_typ{0.0f, 'g', '-', true, enc_type::format_type{}.precision, "0.00000"}

            }));

        auto to_encode = std::get<0>(extent);
        auto format = enc_type::format_type{};
        format.type = static_cast<unsigned char>(std::get<1>(extent));
        format.sign = static_cast<unsigned char>(std::get<2>(extent));
        format.alternate = std::get<3>(extent);
        format.precision = static_cast<std::uint16_t>(std::get<4>(extent));
        std::string_view control = std::get<5>(extent);

        CAPTURE(to_encode, char(format.type), char(format.sign), format.alternate, format.precision);

        enc_type encoder(fstlog::get_default_resource());
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

    SECTION("floating_point_bad_format_type") {
        enc_type encoder(fstlog::get_default_resource());
        std::array<unsigned char, 8> buffer{ '!' };
        fstlog::format_setting_txt format{};
        for (int i = 1; i < 256; i++) {
            const char* good = "aAeEfFgG";
            while (*good != 0 && *good != i) good++;
            if (*good != 0) continue;
            format.type = static_cast<unsigned char>(i);
            buffer.fill('!');
            encoder.output_span_init(buffer);
            encoder.encode(9.123, format);
            auto res = std::string_view(
                reinterpret_cast<char*>(encoder.output_begin()),
                encoder.output_ptr() - encoder.output_begin());
            CAPTURE(i, res);
            CHECK(!encoder.has_error());
            CHECK(res == "9.123");
        }
    };

    SECTION("char") {
        enc_type encoder(fstlog::get_default_resource());
        auto format = enc_type::format_type{};
        format.alternate = true;
        std::array<unsigned char, 256> buffer;
        
        SECTION("default_format") {
            buffer.fill('!');
            encoder.output_span_init(buffer);

            encoder.encode(char{ 'A' }, format);
            encoder.encode(char{ 0 }, format);
            unsigned char byte = 222;
            encoder.encode(*reinterpret_cast<char*>(&byte), format);
            encoder.encode(char16_t{ 'B' }, format);
            encoder.encode(char16_t{ 0xFFFD }, format);
            encoder.encode(char16_t{ 0xFFFF }, format);
            encoder.encode(char32_t{ 'C' }, format);
            encoder.encode(char32_t{ 0xD800 }, format);
            encoder.encode(char32_t{ 0xFFFFFFFF }, format);
            auto res = std::basic_string_view(
                reinterpret_cast<const char*>(encoder.output_begin()),
                encoder.output_ptr() - encoder.output_begin());
            CHECK(!encoder.has_error());
            CHECK(res == "A0x00xdeB0xfffd0xffffC0xd8000xffffffff");
        }

        SECTION("char_format_type") {
            format = fstlog::format_setting_txt{};
            format.alternate = true;
            for (int i = 1; i < 256; i++) {
                format.type = static_cast<unsigned char>(i);
                buffer.fill('!');
                encoder.output_span_init(buffer);
                encoder.encode('J', format);
                auto res = std::string_view(
                    reinterpret_cast<char*>(encoder.output_begin()),
                    encoder.output_ptr() - encoder.output_begin());
                CAPTURE(res);
                CHECK(!encoder.has_error());
                if (i == 0 || i == 'c') {
                    CHECK(res == "J");
                }
                else if (i == 'b') {
                    CHECK(res == "0b1001010");
                }
                else if (i == 'B') {
                    CHECK(res == "0B1001010");
                }
                else if (i == 'x') {
                    CHECK(res == "0x4a");
                }
                else if (i == 'X') {
                    CHECK(res == "0X4A");
                }
                else if (i == 'o') {
                    CHECK(res == "0112");
                }
                else {
                    CHECK(res == "74");
                }
            }
        };

        SECTION("fill_align") {
            buffer.fill('!');
            encoder.output_span_init(buffer);
            format.align = '^';
            format.fill_char[0] = 0xe2;
            format.fill_char[1] = 0x82;
            format.fill_char[2] = 0xb0;
            format.fill_char[3] = 0;
            format.width = 10;

            encoder.encode(char{ 'A' }, format);
            encoder.encode(char{ 0 }, format);
            encoder.encode(char16_t{ 'B' }, format);
            encoder.encode(char16_t{ 0xFFFD }, format);
            encoder.encode(char32_t{ 'C' }, format);
            encoder.encode(char32_t{ 0xD800 }, format);
            auto res = std::basic_string_view(
                reinterpret_cast<const char*>(encoder.output_begin()),
                encoder.output_ptr() - encoder.output_begin());
            CHECK(!encoder.has_error());
            CHECK(res == "₰₰₰₰A₰₰₰₰₰₰₰₰0x0₰₰₰₰₰₰₰₰B₰₰₰₰₰₰₰0xfffd₰₰₰₰₰₰C₰₰₰₰₰₰₰0xd800₰₰");
        }

        SECTION("fill_align_precision") {
            buffer.fill('!');
            encoder.output_span_init(buffer);
            format.align = '>';
            format.fill_char[0] = 0xf0;
            format.fill_char[1] = 0x9f;
            format.fill_char[2] = 0x98;
            format.fill_char[3] = 0x85;
            format.width = 4;
            format.precision = 5;
            
            encoder.encode(char{ 'A' }, format);
            encoder.encode(char{ 0 }, format);
            encoder.encode(char16_t{ u'§' }, format);
            encoder.encode(char16_t{ 0xFFFD }, format);
            encoder.encode(char32_t{ U'®' }, format);
            encoder.encode(char32_t{ 0xD800 }, format);
            auto res = std::basic_string_view(
                reinterpret_cast<const char*>(encoder.output_begin()),
                encoder.output_ptr() - encoder.output_begin());
            CHECK(!encoder.has_error());
            CHECK(res == "😅😅😅A😅0x0😅😅😅§0xfffd😅😅😅®0xd800");
        }
    }

    SECTION("string") {
        enc_type encoder(fstlog::get_default_resource());
        auto format = enc_type::format_type{};
        std::array<unsigned char, 1024> buffer;

        SECTION("default_format") {
            buffer.fill('!');
            encoder.output_span_init(buffer);

            encoder.encode(std::basic_string_view("ASCII string\n"), format);
            encoder.encode(std::string_view("UTF-8 string§©"), format);
            encoder.encode(std::basic_string_view(u"UTF-16 string§©"), format);
            encoder.encode(std::basic_string_view(U"UTF-32 string§©"), format);
            
            auto res = std::basic_string_view(
                reinterpret_cast<const char*>(encoder.output_begin()),
                encoder.output_ptr() - encoder.output_begin());
            CHECK(!encoder.has_error());
            CHECK(res == "ASCII string\\u000AUTF-8 string§©UTF-16 string§©UTF-32 string§©");
        }

        SECTION("fill_align") {
            buffer.fill('!');
            encoder.output_span_init(buffer);
            format.align = '<';
            format.fill_char[0] = 0xc2;
            format.fill_char[1] = 0xa9;
            format.fill_char[2] = 0;
            format.fill_char[3] = 0;
            format.width = 20;

            encoder.encode(std::basic_string_view("ASCII string\n"), format);
            encoder.encode(std::string_view("UTF-8 string§©"), format);
            encoder.encode(std::basic_string_view(u"UTF-16 string§©"), format);
            encoder.encode(std::basic_string_view(U"UTF-32 string§©"), format);

            auto res = std::basic_string_view(
                reinterpret_cast<const char*>(encoder.output_begin()),
                encoder.output_ptr() - encoder.output_begin());
            CHECK(!encoder.has_error());
            CHECK(res == "ASCII string\\u000A©©UTF-8 string§©©©©©©©UTF-16 string§©©©©©©UTF-32 string§©©©©©©");
        }

        SECTION("fill_align_precision") {
            buffer.fill('!');
            encoder.output_span_init(buffer);
            format.align = '^';
            format.fill_char[0] = '.';
            format.fill_char[1] = 0;
            format.fill_char[2] = 0;
            format.fill_char[3] = 0;
            format.width = 10;
            format.precision = 6;

            encoder.encode(std::string_view("ASCII string\n"), format);
            encoder.encode(std::string_view("§©UTF-8 string§©"), format);
            encoder.encode(std::basic_string_view(u"§©UTF-16 string§©"), format);
            encoder.encode(std::basic_string_view(U"§©UTF-32 string§©"), format);

            std::string_view control("..ASCII ....§©UTF-....§©UTF-....§©UTF-..");
            auto len = static_cast<std::size_t>(encoder.output_ptr() - encoder.output_begin());
            CHECK(len == control.size());
            auto res = std::memcmp(encoder.output_begin(), control.data(), len);
            CHECK(res == 0);
        }
    }

    SECTION("string_format_type") {
        enc_type encoder(fstlog::get_default_resource());
        std::array<unsigned char, 8> buffer{ '!' };
        fstlog::format_setting_txt format{};
        for (int i = 1; i < 256; i++) {
            format.type = static_cast<unsigned char>(i);
            buffer.fill('!');
            encoder.output_span_init(buffer);
            encoder.encode(std::string_view("Hello"), format);
            auto res = std::string_view(
                reinterpret_cast<char*>(encoder.output_begin()),
                encoder.output_ptr() - encoder.output_begin());
            CAPTURE(res);
            CHECK(!encoder.has_error());
            CHECK(res == "Hello");
        }
    };

    SECTION("reencode_tail_string") {
        enc_type encoder(fstlog::get_default_resource());
        auto format = enc_type::format_type{};
        std::array<unsigned char, 1024> buffer;
        buffer.fill('!');
        encoder.output_span_init(buffer);


        encoder.encode(std::string_view("String left untouched."), format);
        auto str_begin = encoder.output_ptr();
        encoder.encode(std::string_view("String to \"shift fill\"."), format);
        auto res = std::basic_string_view(
            reinterpret_cast<const char*>(encoder.output_begin()),
            encoder.output_ptr() - encoder.output_begin());
        CHECK(res == "String left untouched.String to \"shift fill\".");

        format.width = 10;
        format.align = '>';
        format.fill_char[0] = 0xc2;
        format.fill_char[1] = 0xa4;
        format.fill_char[2] = 0;
        format.fill_char[3] = 0;
        format.precision = 6;
        encoder.reencode_tail_string(str_begin, format);
        res = std::basic_string_view(
            reinterpret_cast<const char*>(encoder.output_begin()),
            encoder.output_ptr() - encoder.output_begin());
        CHECK(res == "String left untouched.¤¤¤¤String");
    }
}
