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
#include <formatter/impl/logfield_formspec_fmt_mixin.hpp>

using enc_type = fstlog::encoder_stdformat_mixin<
                    fstlog::logfield_formspec_fmt_mixin<
                    fstlog::error_state_mixin<
                    fstlog::output_span_mixin<
                    fstlog::allocator_mixin>>>>;

TEST_CASE("encoder_stdformat_mixin") {
    enc_type encoder;
    std::array<unsigned char, 128> buffer;
    encoder.clear_error();
    encoder.output_span_init(fstlog::byte_span(buffer.data(), buffer.size()));
    encoder.encode(std::string_view{ "string" }, enc_type::format_type{ "{:®<10}" });
    bool stdformat_utf_fill_char = !encoder.has_error();
    
    SECTION("no_space_in_buffer") {
        buffer.fill('!');
        fstlog::byte_span out_buff(buffer.data(), 10);

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

        buffer.fill('!');
        fstlog::byte_span out_buff(buffer.data(), buffer.size());
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

        buffer.fill('!');
        fstlog::byte_span out_buff(buffer.data(), buffer.size());
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

        buffer.fill('!');
        fstlog::byte_span out_buff(buffer.data(), buffer.size());
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

        buffer.fill('!');
        fstlog::byte_span out_buff(buffer.data(), buffer.size());
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
        auto format = enc_type::format_type{"{:c}"};
        buffer.fill('!');

        SECTION("default_format") {
            encoder.clear_error();
            encoder.output_span_init({ buffer.data(), buffer.size() });

            encoder.encode(char{ 'A' }, "{}");
            encoder.encode(char{ 0 }, format);
            unsigned char byte = 222;
            encoder.encode(*reinterpret_cast<char*>(&byte), format);
            encoder.encode(char16_t{ 'B' }, format);
            encoder.encode(char16_t{ 0xFFFD }, format);
            encoder.encode(char16_t{ 0xFFFF }, "{}");
            encoder.encode(char32_t{ 'C' }, format);
            encoder.encode(char32_t{ 0xD800 }, format);
            encoder.encode(char32_t{ 0xFFFFFFFF }, "{}");
            auto res = std::basic_string_view(
                reinterpret_cast<const char*>(encoder.output_begin()),
                encoder.output_ptr() - encoder.output_begin());
            CHECK(!encoder.has_error());
            CHECK(res == "A0deBfffdffffCd800ffffffff");
        }

        SECTION("char_format_type") {
            encoder.clear_error();
            auto data = GENERATE(
                std::make_tuple(std::string_view{"{}"}, std::string_view{ "J" }),
                std::make_tuple(std::string_view{ "{:c}" }, std::string_view{ "J" }),
                std::make_tuple(std::string_view{ "{:d}" }, std::string_view{ "74" }),
                std::make_tuple(std::string_view{ "{:o}" }, std::string_view{ "112" }),
                std::make_tuple(std::string_view{ "{:x}" }, std::string_view{ "4a" }),
                std::make_tuple(std::string_view{ "{:X}" }, std::string_view{ "4A" }),
                std::make_tuple(std::string_view{ "{:b}" }, std::string_view{ "1001010" }),
                std::make_tuple(std::string_view{ "{:B}" }, std::string_view{ "1001010" }),
                std::make_tuple(std::string_view{ "{:#o}" }, std::string_view{ "0112" }),
                std::make_tuple(std::string_view{ "{:#x}" }, std::string_view{ "0x4a" }),
                std::make_tuple(std::string_view{ "{:#X}" }, std::string_view{ "0X4A" }),
                std::make_tuple(std::string_view{ "{:#b}" }, std::string_view{ "0b1001010" }),
                std::make_tuple(std::string_view{ "{:#B}" }, std::string_view{ "0B1001010" })
            );
            
            buffer.fill('!');
            encoder.output_span_init(buffer);
            encoder.encode('J', std::get<0>(data));
            auto res = std::string_view(
                reinterpret_cast<char*>(encoder.output_begin()),
                encoder.output_ptr() - encoder.output_begin());
            CAPTURE(res);
            CHECK(!encoder.has_error());
            CHECK(res == std::get<1>(data));
        };

        SECTION("fill_align") {
            encoder.clear_error();
            buffer.fill('!');
            encoder.output_span_init(buffer);
            format = "{:x^10}";

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
            CHECK(res == "xxxxAxxxxxxxxx0xxxxxxxxxBxxxxxxxxfffdxxxxxxxCxxxxxxxxd800xxx");
        }

        SECTION("fill_align_precision") {
            encoder.clear_error();
            buffer.fill('!');
            encoder.output_span_init(buffer);
            format = "{:x>4.5}";

            encoder.encode(char{ 'A' }, format);
            encoder.encode(char{ 0 }, format);
            encoder.encode(char16_t{ u'q' }, format);
            encoder.encode(char16_t{ 0xFFFD }, format);
            encoder.encode(char32_t{ U'w' }, format);
            encoder.encode(char32_t{ 0xD800 }, format);
            auto res = std::basic_string_view(
                reinterpret_cast<const char*>(encoder.output_begin()),
                encoder.output_ptr() - encoder.output_begin());
            CHECK(!encoder.has_error());
            CHECK(res == "xxxAxxx0xxxqfffdxxxwd800");
        }
    }

    SECTION("string") {
        buffer.fill('!');
        fstlog::byte_span out_buff(buffer.data(), buffer.size());
        auto format = "{:}";
        

        SECTION("default_format") {
            encoder.clear_error();
            encoder.output_span_init(out_buff);

            encoder.encode(std::basic_string_view("ASCII string\n"), format);
            std::string_view str("UTF-8 string§©");
            fstlog::byte_span_const input(reinterpret_cast<const unsigned char*>(str.data()), str.size());
            encoder.encode(input, format);
            encoder.encode(std::basic_string_view(u"UTF-16 string§©"), format);
            encoder.encode(std::basic_string_view(U"UTF-32 string§©"), format);

            auto res = std::basic_string_view(
                reinterpret_cast<const char*>(encoder.output_begin()),
                encoder.output_ptr() - encoder.output_begin());
            CHECK(!encoder.has_error());
            CHECK(res == "ASCII string\\u000AUTF-8 string§©UTF-16 string§©UTF-32 string§©");
        }

        if (stdformat_utf_fill_char) {
            SECTION("fill_align_utf") {
                encoder.clear_error();
                encoder.output_span_init(out_buff);
                format = "{:©<20}";

                encoder.encode(std::basic_string_view("ASCII string\n"), format);
                std::string_view str("UTF-8 string§©");
                fstlog::byte_span_const input(reinterpret_cast<const unsigned char*>(str.data()), str.size());
                encoder.encode(input, format);
                encoder.encode(std::basic_string_view(u"UTF-16 string§©"), format);
                encoder.encode(std::basic_string_view(U"UTF-32 string§©"), format);

                auto res = std::basic_string_view(
                    reinterpret_cast<const char*>(encoder.output_begin()),
                    encoder.output_ptr() - encoder.output_begin());
                CHECK(!encoder.has_error());
                CHECK(res == "ASCII string\\u000A©©UTF-8 string§©©©©©©©UTF-16 string§©©©©©©UTF-32 string§©©©©©©");
            }
        
            SECTION("fill_align_precision_utf") {
                encoder.clear_error();
                encoder.output_span_init(out_buff);
                format = "{:.^10.6}";
            
                encoder.encode(std::basic_string_view("ASCII string\n"), format);
                std::string_view str("§©UTF-8 string§©");
                fstlog::byte_span_const input(reinterpret_cast<const unsigned char*>(str.data()), str.size());
                encoder.encode(input, format);
                encoder.encode(std::basic_string_view(u"§©UTF-16 string§©"), format);
                encoder.encode(std::basic_string_view(U"§©UTF-32 string§©"), format);

                auto res = std::basic_string_view(
                    reinterpret_cast<const char*>(encoder.output_begin()),
                    encoder.output_ptr() - encoder.output_begin());
                CHECK(!encoder.has_error());
                CHECK(res == "..ASCII ....§©UTF-....§©UTF-....§©UTF-..");
            }
        }
        
        SECTION("fill_align") {
            encoder.clear_error();
            encoder.output_span_init(out_buff);
            format = "{:x<20}";

            encoder.encode(std::basic_string_view("ASCII string\n"), format);
            encoder.encode(std::basic_string_view("UTF-8 stringqw"), format);
            encoder.encode(std::basic_string_view(u"UTF-16 stringqw"), format);
            encoder.encode(std::basic_string_view(U"UTF-32 stringqw"), format);

            auto res = std::basic_string_view(
                reinterpret_cast<const char*>(encoder.output_begin()),
                encoder.output_ptr() - encoder.output_begin());
            CHECK(!encoder.has_error());
            CHECK(res == "ASCII string\\u000AxxUTF-8 stringqwxxxxxxUTF-16 stringqwxxxxxUTF-32 stringqwxxxxx");
        }

        SECTION("fill_align_precision") {
            encoder.clear_error();
            encoder.output_span_init(out_buff);
            format = "{:.^10.6}";

            encoder.encode(std::basic_string_view("ASCII string\n"), format);
            encoder.encode(std::basic_string_view("qwUTF-8 stringqw"), format);
            encoder.encode(std::basic_string_view(u"qwUTF-16 stringqw"), format);
            encoder.encode(std::basic_string_view(U"qwUTF-32 stringqw"), format);

            auto res = std::basic_string_view(
                reinterpret_cast<const char*>(encoder.output_begin()),
                encoder.output_ptr() - encoder.output_begin());
            CHECK(!encoder.has_error());
            CHECK(res == "..ASCII ....qwUTF-....qwUTF-....qwUTF-..");
        }
    }

    SECTION("reencode_tail_string") {
        buffer.fill('!');
        fstlog::byte_span out_buff(buffer.data(), buffer.size());
        auto format = "{:}";
        encoder.clear_error();
        encoder.output_span_init(out_buff);

        encoder.encode(std::string_view("String left untouched."), format);
        auto str_begin = encoder.output_ptr();
        encoder.encode(std::string_view("String to \"shift fill\"."), format);
        auto res = std::basic_string_view(
            reinterpret_cast<const char*>(encoder.output_begin()),
            encoder.output_ptr() - encoder.output_begin());
        CHECK(res == "String left untouched.String to \"shift fill\".");
        
        format = "{:x>10.6}";
        encoder.reencode_tail_string(str_begin, format);
        res = std::basic_string_view(
            reinterpret_cast<const char*>(encoder.output_begin()),
            encoder.output_ptr() - encoder.output_begin());
        CHECK(res == "String left untouched.xxxxString");
        
        if (stdformat_utf_fill_char) {
            format = "{:¤>10.6}";
            encoder.reencode_tail_string(str_begin, format);
            res = std::basic_string_view(
                reinterpret_cast<const char*>(encoder.output_begin()),
                encoder.output_ptr() - encoder.output_begin());
            CHECK(res == "String left untouched.¤¤¤¤xxxxSt");
        }
    }
}
#endif
