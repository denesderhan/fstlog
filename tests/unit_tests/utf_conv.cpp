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

TEST_CASE("valid_utf_char") {
    SECTION("valid_utf_char<32>") {
        CHECK(fstlog::detail::utf::valid_utf_char<32>((std::numeric_limits<std::int32_t>::min)()) == false);
        CHECK(fstlog::detail::utf::valid_utf_char<32>(std::int32_t(-1)) == false);
        CHECK(fstlog::detail::utf::valid_utf_char<32>(std::int32_t(0)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<32>(std::int32_t(10)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<32>((std::numeric_limits<std::int32_t>::max)()) == true);

        CHECK(fstlog::detail::utf::valid_utf_char<32>((std::numeric_limits<std::uint32_t>::min)()) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<32>(std::uint32_t(10)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<32>((std::numeric_limits<std::uint32_t>::max)()) == true);

        CHECK(fstlog::detail::utf::valid_utf_char<32>((std::numeric_limits<std::int64_t>::min)()) == false);
        CHECK(fstlog::detail::utf::valid_utf_char<32>(std::int64_t(-1)) == false);
        CHECK(fstlog::detail::utf::valid_utf_char<32>(std::int64_t(0)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<32>(std::int64_t(10)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<32>(std::int64_t(0xFFFF'FFFFU)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<32>(std::int64_t(0x1'0000'0000U)) == false);
        CHECK(fstlog::detail::utf::valid_utf_char<32>((std::numeric_limits<std::int64_t>::max)()) == false);

        CHECK(fstlog::detail::utf::valid_utf_char<32>((std::numeric_limits<std::uint64_t>::min)()) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<32>(std::uint64_t(10)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<32>(std::uint64_t(0xFFFF'FFFFU)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<32>(std::uint64_t(0x1'0000'0000U)) == false);
        CHECK(fstlog::detail::utf::valid_utf_char<32>((std::numeric_limits<std::uint64_t>::max)()) == false);
    }

    SECTION("valid_utf_char<16>") {
        CHECK(fstlog::detail::utf::valid_utf_char<16>((std::numeric_limits<std::int16_t>::min)()) == false);
        CHECK(fstlog::detail::utf::valid_utf_char<16>(std::int16_t(-1)) == false);
        CHECK(fstlog::detail::utf::valid_utf_char<16>(std::int16_t(0)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<16>(std::int16_t(10)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<16>((std::numeric_limits<std::int16_t>::max)()) == true);

        CHECK(fstlog::detail::utf::valid_utf_char<16>((std::numeric_limits<std::uint16_t>::min)()) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<16>(std::uint16_t(10)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<16>((std::numeric_limits<std::uint16_t>::max)()) == true);

        CHECK(fstlog::detail::utf::valid_utf_char<16>((std::numeric_limits<std::int64_t>::min)()) == false);
        CHECK(fstlog::detail::utf::valid_utf_char<16>(std::int64_t(-1)) == false);
        CHECK(fstlog::detail::utf::valid_utf_char<16>(std::int64_t(0)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<16>(std::int64_t(10)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<16>(std::int64_t(0xFFFFU)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<16>(std::int64_t(0x1'0000U)) == false);
        CHECK(fstlog::detail::utf::valid_utf_char<16>((std::numeric_limits<std::int64_t>::max)()) == false);

        CHECK(fstlog::detail::utf::valid_utf_char<16>((std::numeric_limits<std::uint64_t>::min)()) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<16>(std::uint64_t(10)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<16>(std::uint64_t(0xFFFFU)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<16>(std::uint64_t(0x1'0000U)) == false);
        CHECK(fstlog::detail::utf::valid_utf_char<16>((std::numeric_limits<std::uint64_t>::max)()) == false);
    }

    SECTION("valid_utf_char<8>") {
        CHECK(fstlog::detail::utf::valid_utf_char<8>((std::numeric_limits<std::int8_t>::min)()) == false);
        CHECK(fstlog::detail::utf::valid_utf_char<8>(std::int8_t(-1)) == false);
        CHECK(fstlog::detail::utf::valid_utf_char<8>(std::int8_t(0)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<8>(std::int8_t(10)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<8>((std::numeric_limits<std::int8_t>::max)()) == true);

        CHECK(fstlog::detail::utf::valid_utf_char<8>((std::numeric_limits<std::uint8_t>::min)()) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<8>(std::uint8_t(10)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<8>((std::numeric_limits<std::uint8_t>::max)()) == true);

        CHECK(fstlog::detail::utf::valid_utf_char<8>((std::numeric_limits<std::int64_t>::min)()) == false);
        CHECK(fstlog::detail::utf::valid_utf_char<8>(std::int64_t(-1)) == false);
        CHECK(fstlog::detail::utf::valid_utf_char<8>(std::int64_t(0)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<8>(std::int64_t(10)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<8>(std::int64_t(0xFFU)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<8>(std::int64_t(0x100U)) == false);
        CHECK(fstlog::detail::utf::valid_utf_char<8>((std::numeric_limits<std::int64_t>::max)()) == false);

        CHECK(fstlog::detail::utf::valid_utf_char<8>((std::numeric_limits<std::uint64_t>::min)()) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<8>(std::uint64_t(10)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<8>(std::uint64_t(0xFFU)) == true);
        CHECK(fstlog::detail::utf::valid_utf_char<8>(std::uint64_t(0x100U)) == false);
        CHECK(fstlog::detail::utf::valid_utf_char<8>((std::numeric_limits<std::uint64_t>::max)()) == false);
    }
}

TEST_CASE("valid_utf_code_point") {
    CHECK(fstlog::detail::utf::valid_utf_code_point(std::uint32_t{ 0 }) == true);
    CHECK(fstlog::detail::utf::valid_utf_code_point(std::uint32_t{ 0xD7FF }) == true);
    CHECK(fstlog::detail::utf::valid_utf_code_point(std::uint32_t{ 0xD800 }) == false);
    CHECK(fstlog::detail::utf::valid_utf_code_point(std::uint32_t{ 0xD801 }) == false);
    CHECK(fstlog::detail::utf::valid_utf_code_point(std::uint32_t{ 0xDFFE }) == false);
    CHECK(fstlog::detail::utf::valid_utf_code_point(std::uint32_t{ 0xDFFF }) == false);
    CHECK(fstlog::detail::utf::valid_utf_code_point(std::uint32_t{ 0xE000 }) == true);
    CHECK(fstlog::detail::utf::valid_utf_code_point(std::uint32_t{ 0xE001 }) == true);
    CHECK(fstlog::detail::utf::valid_utf_code_point(std::uint32_t{ 0x10000 }) == true);
    CHECK(fstlog::detail::utf::valid_utf_code_point(std::uint32_t{ 0x10FFFF }) == true);
    CHECK(fstlog::detail::utf::valid_utf_code_point(std::uint32_t{ 0x110000 }) == false);
    CHECK(fstlog::detail::utf::valid_utf_code_point(std::uint32_t{ 0xFFFFFFFF }) == false);
}

TEST_CASE("safe_utf_code_point") {
    SECTION("control_range") {
        // C0
        for (std::uint32_t codepoint = 0x00; codepoint <= 0x1F; codepoint++) {
            CHECK(fstlog::detail::utf::safe_utf_code_point(codepoint) == false);
        }
        // C1
        for (std::uint32_t codepoint = 0x7F; codepoint <= 0x9F; codepoint++) {
            CHECK(fstlog::detail::utf::safe_utf_code_point(codepoint) == false);
        }
        // no break space is unsafe
        CHECK(fstlog::detail::utf::safe_utf_code_point(0xA0) == false);
    };

    SECTION("surrogate_range") {
        CHECK(fstlog::detail::utf::safe_utf_code_point(0xD800) == false);
        CHECK(fstlog::detail::utf::safe_utf_code_point(0xDBFF) == false);
        CHECK(fstlog::detail::utf::safe_utf_code_point(0xDC00) == false);
        CHECK(fstlog::detail::utf::safe_utf_code_point(0xDFFF) == false);
    }

    SECTION("char_range") {

        std::vector<std::uint32_t> codepoints{
            //false, true,  true,   false   
            0x001F, 0x0020, 0x007E, 0x007F,     // Printable ASCII
            0x00A0, 0x00A1, 0x02AF, 0x02B0,     // Latin-1 Supplement, extended, IPA
            0x036F, 0x0370, 0x06FF, 0x0700,     // Greek, Cyrillic, Armenian, Hebrew, Arabic
            0x08FF, 0x0900, 0x097F, 0x0980,     // Devanagari
            0x209F, 0x20A0, 0x20BF, 0x20C0,     // Currency
            0x3000, 0x3001, 0x303D, 0x303E,     // Hiragana 
            0x3040, 0x3041, 0x30FF, 0x3100,     // Katakana
            0x32FF, 0x3300, 0xA4C6, 0xA4C7,     // CJK, YI
            0xABFF, 0xAC00, 0xD7A3, 0xD7A4,     // Hangul Syllables
            0x1F2FF, 0x1F300, 0x1F3FA, 0x1F3FB, // Misc symbols
            0x1F3FF, 0x1F400, 0x1F6D7, 0x1F6D8  // Misc symbols, Emoticons
        };

        std::vector<bool> control{
            false, true, true, false,
            false, true, true, false,
            false, true, true, false,
            false, true, true, false,
            false, true, true, false,
            false, true, true, false,
            false, true, true, false,
            false, true, true, false,
            false, true, true, false,
            false, true, true, false,
            false, true, true, false
        };
        REQUIRE(control.size() == codepoints.size());
        std::size_t ind{ 0 };
        for (auto codep : codepoints) {
            CHECK(fstlog::detail::utf::safe_utf_code_point(codep) == control[ind]);
            ind++;
        }
    }
}

TEST_CASE("encode_escaped") {
    std::array<unsigned char, 16> buffer;
    
    SECTION("regular") {
        auto data = GENERATE(
            std::make_tuple(std::uint32_t{ 0 }, "\\u0000"),
            std::make_tuple(std::uint32_t{ 0xFFFF }, "\\uFFFF"),
            std::make_tuple(std::uint32_t{ 0x10000 }, "\\U00010000"),
            std::make_tuple(std::uint32_t{ 0x10FFFF }, "\\U0010FFFF"),
            std::make_tuple(std::uint32_t{ 0x09ABCD }, "\\U0009ABCD"),
            std::make_tuple(std::uint32_t{ 0xABCD }, "\\uABCD"));
        
        buffer.fill('!');
        auto code_point = std::get<0>(data);
        std::string_view control = std::get<1>(data);
        fstlog::unaligned_span buff(buffer);
        auto result = fstlog::detail::utf::encode_escaped(code_point, buff);
        const std::size_t len = code_point <= 0xFFFF ? 6 : 10;
        CHECK(result == len);
        CHECK(buff.data_bytes() == buffer.data() + len);
        CHECK(std::string_view(reinterpret_cast<const char*>(buffer.data()), len) == control);
    }
}

TEST_CASE("encode_safe_utf8_char") {
    std::array<unsigned char, 300> buffer{ 0 };

    SECTION("regular") {
        std::vector<std::uint32_t> data{
            0x0000, 0x0008, 0x000B, 0x000D,     //ASCII control
         //escaped, char,  char,   escaped   
            0x001F, 0x0020, 0x007E, 0x007F,     // Printable ASCII
            0x00A0, 0x00A1, 0x02AF, 0x02B0,     // Latin-1 Supplement, extended, IPA
            0x036F, 0x0370, 0x06FF, 0x0700,     // Greek, Cyrillic, Armenian, Hebrew, Arabic
            0x08FF, 0x0900, 0x097F, 0x0980,     // Devanagari
            0x209F, 0x20A0, 0x20BF, 0x20C0,     // Currency
            0x3000, 0x3001, 0x303D, 0x303E,     // Hiragana 
            0x3040, 0x3041, 0x30FF, 0x3100,     // Katakana
            0x32FF, 0x3300, 0xA4C6, 0xA4C7,     // CJK, YI
            0xABFF, 0xAC00, 0xD7A3, 0xD7A4,     // Hangul Syllables
            0x1F2FF, 0x1F300, 0x1F3FA, 0x1F3FB, // Misc symbols
            0x1F3FF, 0x1F400, 0x1F6D7, 0x1F6D8  // Misc symbols, Emoticons
        };

        const unsigned char control[] = 
            "\\u0000\\u0008\\u000B\\u000D" //ASCII control
            "\\u001F ~\\u007F" // Printable ASCII
            "\\u00A0¡ʯ\\u02B0" // Latin-1 Supplement, extended, IPA
            "\\u036FͰۿ\\u0700" // Greek, Cyrillic, Armenian, Hebrew, Arabic
            "\\u08FFऀॿ\\u0980" // Devanagari
            "\\u209F₠₿\\u20C0" // Currency
            "\\u3000、〽\\u303E" // Hiragana, Katagana
            "\\u3040ぁヿ\\u3100" // Hiragana, Katagana
            "\\u32FF㌀꓆\\uA4C7" // CJK, YI
            "\\uABFF가힣\\uD7A4" // Hangul Syllables
            "\\U0001F2FF🌀🏺\\U0001F3FB" // Misc symbols
            "\\U0001F3FF🐀🛗\\U0001F6D8"; // Misc symbols, Emoticons
        
        fstlog::byte_span output(buffer);
        for (auto codepoint : data) {
            fstlog::detail::utf::encode_safe_utf8_char(codepoint, output);
        }
        const auto len = buffer.size() - output.size();
        CHECK(len == sizeof(control) - 1);
        auto res = std::memcmp(control, buffer.data(), len);
        CHECK( res == 0);
    }
}

TEST_CASE("decode_utf16_char") {
    SECTION("valid_1") {
        std::array<char16_t, 2> buffer{ 'a', 0x20AC };
        fstlog::unaligned_span<const char16_t> buff(buffer);
        
        auto code_point = fstlog::detail::utf::decode_utf16_char<char16_t>(buff);
        CHECK(buff.size() == 1);
        CHECK(code_point == 'a');
                
        code_point = fstlog::detail::utf::decode_utf16_char<char16_t>(buff);
        CHECK(buff.size() == 0);
        CHECK(code_point == 0x20AC);
    }
    SECTION("missing_surrogate") {
        std::array<char16_t, 1> buffer{ 0xD83D };
        fstlog::unaligned_span<const char16_t> buff(buffer);
        auto code_point = fstlog::detail::utf::decode_utf16_char<char16_t>(buff);
        CHECK(buff.size() == 0);
        CHECK(code_point == 0xFFFD);
    }
    
    SECTION("surrogate_error") {
        auto data = GENERATE(
            std::array<char16_t, 2>{ 0xDE00, 0xDE00 }, // invalid_1st_surrogate
            std::array<char16_t, 2>{ 0xD83D, 0x0 }, // invalid_2nd_surrogate
            std::array<char16_t, 2>{ 0xD83D, 0xD83D },
            std::array<char16_t, 2>{ 0xD83D, 0xFFFF },
            std::array<char16_t, 2>{ 0xDE00, 0xD83D }, // invalid_1_2_surrogate
            std::array<char16_t, 2>{ 0xDC00, 0xDB7F }
        );

        fstlog::unaligned_span<const char16_t> buff(data);
        auto code_point = fstlog::detail::utf::decode_utf16_char<char16_t>(buff);
        CHECK(buff.size() == 1);
        CHECK(code_point == 0xFFFD);
    }
    
    SECTION("valid_surrogates") {
        std::tuple<std::array<char16_t, 2>, char32_t> data = GENERATE(
            std::tuple<std::array<char16_t, 2>, char32_t>{ std::array<char16_t, 2>{0xD83D, 0xDE00}, 0x1F600u },
            std::tuple<std::array<char16_t, 2>, char32_t>{ std::array<char16_t, 2>{0xDB7F, 0xDC00}, 0xEFC00u }
        );
        fstlog::unaligned_span<const char16_t> buff(std::get<0>(data));
        auto code_point = fstlog::detail::utf::decode_utf16_char<char16_t>(buff);
        CHECK(buff.size() == 0);
        CHECK(code_point == std::get<1>(data));
    }
}

TEST_CASE("utfX_to_utfY") {
    constexpr unsigned char u8_text[] = {
        "UTF encoding is a standard for encoding characters in the Unicode character set."
        "L'encodage UTF est une norme pour l'encodage des caractères dans l'ensemble de caractères Unicode."
        "UTF kódování je standard pro kódování znaků v sadě znaků Unicode."
        "Kodowanie UTF jest standardem kodowania znaków w zestawie znaków Unicode."
        "UTF-kodning er en standard for kodning af tegn i Unicode-tegnsættet."
        "Η κωδικοποίηση UTF είναι ένα πρότυπο για την κωδικοποίηση χαρακτήρων στο σύνολο χαρακτήρων Unicode."
        "La codificación UTF es un estándar para codificar caracteres en el conjunto de caracteres Unicode."
        "Кодировка UTF является стандартом для кодирования символов в наборе символов Unicode."
        "ترميز UTF هو معيار لترميز الأحرف في مجموعة أحرف يونيكود."
        "UTF एन्कोडिंग यूनिकोड कैरेक्टर सेट में कैरेक्टर को एन्कोड करने के लिए एक मानक है।"
        "UTFエンコーディングは、Unicode文字セット内の文字をエンコードするための標準です。"
        "UTF 编码者, Unicode 字符集字符编码之准也。"
        "UTF 인코딩은 유니코드 문자 집합에서 문자를 인코딩하는 표준입니다."
        "😀🙏🚗"
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
        u"ترميز UTF هو معيار لترميز الأحرف في مجموعة أحرف يونيكود." // arabic
        u"UTF एन्कोडिंग यूनिकोड कैरेक्टर सेट में कैरेक्टर को एन्कोड करने के लिए एक मानक है।" // devangari
        u"UTFエンコーディングは、Unicode文字セット内の文字をエンコードするための標準です。" // japanese
        u"UTF 编码者, Unicode 字符集字符编码之准也。" // chinese
        u"UTF 인코딩은 유니코드 문자 집합에서 문자를 인코딩하는 표준입니다." // korean
        u"😀🙏🚗"
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
        U"UTFエンコーディングは、Unicode文字セット内の文字をエンコードするための標準です。"
        U"UTF 编码者, Unicode 字符集字符编码之准也。"
        U"UTF 인코딩은 유니코드 문자 집합에서 문자를 인코딩하는 표준입니다."
        U"😀🙏🚗"
    };
    constexpr auto text_char_num{ u32_text.size() };

    SECTION("utf32_to_utf8") {
        std::size_t wanted_char_num = GENERATE(0, 1, 10, 530, 917, (std::numeric_limits<std::size_t>::max)());
        CAPTURE(wanted_char_num);
        std::vector<unsigned char> out_buff(4096, 0);
        fstlog::unaligned_span<unsigned char> output(out_buff.data(), out_buff.size());
        fstlog::unaligned_span<const char32_t> input(u32_text.data(), u32_text.size());
        const auto result = fstlog::detail::utf::utf32_to_utf8<char32_t>(
            input,
            output,
            wanted_char_num);
        CHECK(result.ec == fstlog::error_code::none);
        const auto [trimmed_len, trimmed_num] = fstlog::detail::utf::utf8_str_trim(
            fstlog::byte_span_const(&u8_text[0], sizeof(u8_text) - 1),
            wanted_char_num);
        if (wanted_char_num == (std::numeric_limits<std::size_t>::max)())
            wanted_char_num = text_char_num;
        CHECK(trimmed_num == wanted_char_num);
        CHECK(trimmed_len == out_buff.size() - output.size());
        auto res = std::memcmp(out_buff.data(), &u8_text[0], trimmed_len);
        CHECK(res == 0);
    };

    SECTION("utf16_to_utf8") {
        std::size_t wanted_char_num = GENERATE(0, 1, 10, 530, 917, (std::numeric_limits<std::size_t>::max)());
        CAPTURE(wanted_char_num);
        std::vector<unsigned char> out_buff(4096, 0);
        fstlog::unaligned_span<unsigned char> output(out_buff.data(), out_buff.size());
        fstlog::unaligned_span<const char16_t> input(u16_text.data(), u16_text.size());
        const auto result = fstlog::detail::utf::utf16_to_utf8<char16_t>(
            input,
            output,
            wanted_char_num);
        CHECK(result.ec == fstlog::error_code::none);
        const auto [trimmed_len, trimmed_num] = fstlog::detail::utf::utf8_str_trim(
            fstlog::byte_span_const(&u8_text[0], sizeof(u8_text) - 1),
            wanted_char_num);
        if (wanted_char_num == (std::numeric_limits<std::size_t>::max)())
            wanted_char_num = text_char_num;
        CHECK(trimmed_num == wanted_char_num);
        CHECK(trimmed_len == out_buff.size() - output.size());
        auto res = std::memcmp(out_buff.data(), &u8_text[0], trimmed_len);
        CHECK(res == 0);
    };

    SECTION("utf8_to_utf8") {
        std::size_t wanted_char_num = GENERATE(0, 1, 10, 530, 917, (std::numeric_limits<std::size_t>::max)());
        CAPTURE(wanted_char_num);
        std::vector<unsigned char> out_buff(4096, 0);
        fstlog::byte_span output(out_buff.data(), out_buff.size());
        fstlog::byte_span_const input(&u8_text[0], sizeof(u8_text) - 1);
        const auto result = fstlog::detail::utf::utf8_to_utf8(
            input,
            output,
            wanted_char_num);
        CHECK(result.ec == fstlog::error_code::none);
        const auto [trimmed_len, trimmed_num] = fstlog::detail::utf::utf8_str_trim(
            fstlog::byte_span_const(&u8_text[0], sizeof(u8_text) - 1),
            wanted_char_num);
        if (wanted_char_num == (std::numeric_limits<std::size_t>::max)())
            wanted_char_num = text_char_num;
        CHECK(trimmed_num == wanted_char_num);
        CHECK(trimmed_len == out_buff.size() - output.size());
        auto res = std::memcmp(out_buff.data(), &u8_text[0], trimmed_len);
        CHECK(res == 0);
    };

    SECTION("utf32_to_utf8_irregular") {
        auto test_data = GENERATE(
            std::make_tuple(std::vector<char32_t>{'S', 'u', 'r', 'r', 'o', 'g', 'a', 't', 'e', ':', 0xD800, 0xDA00, 0xDFFF}, "Surrogate:\\uFFFD\\uFFFD\\uFFFD"),
            std::make_tuple(std::vector<char32_t>{'O', 'u', 't', ':', 0x00110000, 0x001FFFFF, 0xFFFFFFFF}, "Out:\\uFFFD\\uFFFD\\uFFFD"),
            std::make_tuple(std::vector<char32_t>{'C', '0', ':', 0x0000, 0x000A, 0x000C, 0x001F, 0x007F}, "C0:\\u0000\\u000A\\u000C\\u001F\\u007F"),
            std::make_tuple(std::vector<char32_t>{'C', '1', ':', 0x0080, 0x008C, 0x009F}, "C1:\\u0080\\u008C\\u009F"),
            std::make_tuple(std::vector<char32_t>{'U', 'n', 's', 'a', 'f', 'e', ':', 0x02B0, 0x20D0}, "Unsafe:\\u02B0\\u20D0")
        );
        
        auto &in_data = std::get<0>(test_data);
        auto control_txt = std::get<1>(test_data);
        std::vector<unsigned char> out_buff(2048, 0);
        fstlog::byte_span output(out_buff.data(), out_buff.size());
        fstlog::unaligned_span<const char32_t> input(in_data.data(), in_data.size());
        const auto result = fstlog::detail::utf::utf32_to_utf8(input, output);
        CHECK(result.ec == fstlog::error_code::none);
        auto len = out_buff.size() - output.size();
        CHECK(len == std::strlen(control_txt));
        auto res = std::memcmp(control_txt, out_buff.data(), len);
        CHECK(res == 0);
    };

    SECTION("utf16_to_utf8_irregular") {
        auto test_data = GENERATE(
            std::make_tuple(std::vector<char16_t>{'S', 'u', 'r', 'r', 'o', 'g', 'a', 't', 'e', ':', 0xD800, '_', 0xDA00, '_', 0xDFFF}, "Surrogate:\\uFFFD_\\uFFFD_\\uFFFD"),
            std::make_tuple(std::vector<char16_t>{'C', '0', ':', 0x0000, 0x000A, 0x000C, 0x001F, 0x007F}, "C0:\\u0000\\u000A\\u000C\\u001F\\u007F"),
            std::make_tuple(std::vector<char16_t>{'C', '1', ':', 0x0080, 0x008C, 0x009F}, "C1:\\u0080\\u008C\\u009F"),
            std::make_tuple(std::vector<char16_t>{'U', 'n', 's', 'a', 'f', 'e', ':', 0x02B0, 0x20D0 }, "Unsafe:\\u02B0\\u20D0")
        );

        auto & in_data = std::get<0>(test_data);
        auto control_txt = std::get<1>(test_data);
        std::vector<unsigned char> out_buff(2048, 0);
        fstlog::byte_span output(out_buff.data(), out_buff.size());
        fstlog::unaligned_span<const char16_t> input(in_data.data(), in_data.size());
        const auto result = fstlog::detail::utf::utf16_to_utf8(input, output);
        CHECK(result.ec == fstlog::error_code::none);
        auto len = out_buff.size() - output.size();
        CHECK(len == std::strlen(control_txt));
        auto res = std::memcmp(control_txt, out_buff.data(), len);
        CHECK(res == 0);
    };

    SECTION("utf8_to_utf8_irregular") {
        auto test_data = GENERATE(
            std::make_tuple(std::vector<unsigned char>{'S', 'u', 'r', 'r', 'o', 'g', 'a', 't', 'e', ':', 0xED, 0xA0, 0x80, 0xED, 0xA8, 0x80, 0xED, 0xBF, 0xBF}, "Surrogate:\\uFFFD\\uFFFD\\uFFFD"),
            std::make_tuple(std::vector<unsigned char>{'C', '0', ':', 0x0000, 0x000A, 0x000C, 0x001F, 0x007F}, "C0:\\u0000\\u000A\\u000C\\u001F\\u007F"),
            std::make_tuple(std::vector<unsigned char>{'C', '1', ':', 0xC2, 0x80, 0xC2, 0x8C, 0xC2, 0x9F}, "C1:\\u0080\\u008C\\u009F"),
            std::make_tuple(std::vector<unsigned char>{'U', 'n', 's', 'a', 'f', 'e', ':', 0xCA, 0xB0, 0xE2, 0x83, 0x90 }, "Unsafe:\\u02B0\\u20D0")
        );

        auto &in_data = std::get<0>(test_data);
        auto control_txt = std::get<1>(test_data);
        std::vector<unsigned char> out_buff(2048, 0);
        fstlog::byte_span output(out_buff.data(), out_buff.size());
        fstlog::byte_span_const input(in_data.data(), in_data.size());
        const auto result = fstlog::detail::utf::utf8_to_utf8(input, output);
        CHECK(result.ec == fstlog::error_code::none);
        auto len = out_buff.size() - output.size();
        CHECK(len == std::strlen(control_txt));
        auto res = std::memcmp(control_txt, out_buff.data(), len);
        CHECK(res == 0);
    };

    SECTION("utf32_to_utf8_no_space_in_buffer") {
        std::basic_string_view str{U"UTF 编码者"};
        std::array<unsigned char, 13> buffer;
        buffer.fill('!');
        fstlog::byte_span output(buffer);
        fstlog::unaligned_span input(str.data(), str.size());
           const auto result = fstlog::detail::utf::utf32_to_utf8(input, output);
        CHECK(result.ec == fstlog::error_code::buff_full);
        CHECK(output.size() == 9);
        CHECK(result.codepoints_written == 4);
        std::string_view control{ "UTF " };
        CHECK(std::string_view(reinterpret_cast<const char*>(
            buffer.data()), buffer.size() - output.size()) == control);
    }

    SECTION("utf16_to_utf8_no_space_in_buffer") {
        std::basic_string_view str{ u"UTF 编码者" };
        std::array<unsigned char, 13> buffer;
        buffer.fill('!');
        fstlog::byte_span output(buffer);
        fstlog::unaligned_span input(str.data(), str.size());
        const auto result = fstlog::detail::utf::utf16_to_utf8(input, output);
        CHECK(result.ec == fstlog::error_code::buff_full);
        CHECK(output.size() == 9);
        CHECK(result.codepoints_written == 4);
        std::string_view control{ "UTF " };
        CHECK(std::string_view(reinterpret_cast<const char*>(
            buffer.data()), buffer.size() - output.size()) == control);
    }

    SECTION("utf8_to_utf8_no_space_in_buffer") {
        std::string_view str{ "UTF 编码者" };
        std::array<unsigned char, 13> buffer;
        buffer.fill('!');
        fstlog::byte_span output(buffer);
        fstlog::byte_span_const input(reinterpret_cast<const unsigned char*>(str.data()), str.size());
        const auto result = fstlog::detail::utf::utf8_to_utf8(input, output);
        CHECK(result.ec == fstlog::error_code::buff_full);
        CHECK(output.size() == 9);
        CHECK(result.codepoints_written == 4);
        std::string_view control{ "UTF " };
        CHECK(std::string_view(reinterpret_cast<const char*>(
            buffer.data()), buffer.size() - output.size()) == control);
    }
}

TEST_CASE("decode_utf8_char_benchmark", "[.][benchmark]") {
    auto input = GENERATE(
        "UTF encoding is a standard for encoding characters in the Unicode character set.",
        "L'encodage UTF est une norme pour l'encodage des caractères dans l'ensemble de caractères Unicode.",
        "UTF kódování je standard pro kódování znaků v sadě znaků Unicode.",
        "Kodowanie UTF jest standardem kodowania znaków w zestawie znaków Unicode.",
        "UTF-kodning er en standard for kodning af tegn i Unicode-tegnsættet.",
        "Η κωδικοποίηση UTF είναι ένα πρότυπο για την κωδικοποίηση χαρακτήρων στο σύνολο χαρακτήρων Unicode.",
        "La codificación UTF es un estándar para codificar caracteres en el conjunto de caracteres Unicode.",
        "Кодировка UTF является стандартом для кодирования символов в наборе символов Unicode.",
        "ترميز UTF هو معيار لترميز الأحرف في مجموعة أحرف يونيكود.",
        "UTF एन्कोडिंग यूनिकोड कैरेक्टर सेट में कैरेक्टर को एन्कोड करने के लिए एक मानक है।",
        "UTFエンコーディングは、Unicode文字セット内の文字をエンコードするための標準です。",
        "UTF 编码者, Unicode 字符集字符编码之准也。",
        "UTF 인코딩은 유니코드 문자 집합에서 문자를 인코딩하는 표준입니다."
    );

    DYNAMIC_SECTION("input_" << reinterpret_cast<const char*>(input)) {
        const unsigned char* const begin = reinterpret_cast<const unsigned char*>(input);
        std::size_t size = strlen(input);
        BENCHMARK_ADVANCED("decode_utf8_char")(Catch::Benchmark::Chronometer meter) {
            meter.measure([begin, size] {
                fstlog::unaligned_span<const unsigned char> input(begin, size);
                std::size_t res = 0;
                while (!input.empty()) {
                    res += fstlog::detail::utf::decode_utf8_char(input);
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
            fstlog::byte_span_const input(&c, 1);
            auto codep = fstlog::detail::utf::decode_utf8_char(input);
            CHECK(c == codep );
            CHECK(input.empty());
        }
        // invalid encoding
        for (unsigned char c = 0xFF; c >= 0x80; c--) {
            fstlog::byte_span_const input(&c, 1);
            auto codep = fstlog::detail::utf::decode_utf8_char(input);
            CHECK(0xFFFD == codep);
            CHECK(input.empty());
        }
    }

    SECTION("two_byte") {
        // 0b110x'xxxx, 0b10yy'yyyy
        auto test = GENERATE(
            // valid encoded
            std::make_tuple(std::array<unsigned char, 2>{ 0b1100'0010, 0b1000'0000}, 2, 0, 0x80U),
            std::make_tuple(std::array<unsigned char, 2>{ 0b1101'0000, 0b1010'0000}, 2, 0, 0x420U),
            std::make_tuple(std::array<unsigned char, 2>{ 0b1101'1111, 0b1011'1111}, 2, 0, 0x7FFU),
            // overlong encoded
            std::make_tuple(std::array<unsigned char, 2>{ 0b1100'0000, 0b1000'0000}, 2, 0, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 2>{ 0b1100'0001, 0b1011'1111}, 2, 0, 0xFFFDU),
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
            std::make_tuple(std::array<unsigned char, 2>{ 0b1100'0010, 0b1000'0000}, 1, 0, 0xFFFDU)
        );

        auto& arr = std::get<0>(test);
        fstlog::byte_span_const input(arr.data(), std::get<1>(test));

        auto codep = fstlog::detail::utf::decode_utf8_char(input);

        CHECK(codep == std::get<3>(test));
        CHECK(input.size() == std::get<2>(test));
    }

    SECTION("three_byte") {
        // 0b1110'xxxx, 0b10yy'yyyy, 0b10zz'zzzz
        auto test = GENERATE(
            // valid encoded
            std::make_tuple(std::array<unsigned char, 3>{ 0b1110'0000, 0b1010'0000, 0b1000'0000}, 3, 0, 0x800U),
            std::make_tuple(std::array<unsigned char, 3>{ 0b1110'0000, 0b1010'0000, 0b1010'0000}, 3, 0, 0x820U),
            std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1111, 0b1011'1111, 0b1011'1111}, 3, 0, 0xFFFFU),
            std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1111, 0b1011'1111, 0b1011'1101}, 3, 0, 0xFFFDU),
            // overlong encoded
            std::make_tuple(std::array<unsigned char, 3>{ 0b1110'0000, 0b1000'0000, 0b1000'0000}, 3, 0, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 3>{ 0b1110'0000, 0b1001'1111, 0b1011'1111}, 3, 0, 0xFFFDU),
            // invalid first byte
            std::make_tuple(std::array<unsigned char, 3>{ 0b1000'0000, 0b1010'0000, 0b1000'0000}, 3, 2, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 3>{ 0b1001'0000, 0b1010'0000, 0b1010'0000}, 3, 2, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 3>{ 0b1010'1111, 0b1011'1111, 0b1011'1111}, 3, 2, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 3>{ 0b1000'0000, 0b1010'0000, 0b1000'0000}, 3, 2, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 3>{ 0b1011'0000, 0b1010'0000, 0b1010'0000}, 3, 2, 0xFFFDU),
            // invalid second byte
            std::make_tuple(std::array<unsigned char, 3>{ 0b1110'0000, 0b1110'0000, 0b1000'0000}, 3, 2, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 3>{ 0b1110'0000, 0b0110'0000, 0b1010'0000}, 3, 2, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1111, 0b0011'1111, 0b1011'1111}, 3, 2, 0xFFFDU),
            // invalid third byte
            std::make_tuple(std::array<unsigned char, 3>{ 0b1110'0000, 0b1010'0000, 0b1100'0000}, 3, 1, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 3>{ 0b1110'0000, 0b1010'0000, 0b0010'0000}, 3, 1, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1111, 0b1011'1111, 0b0011'1111}, 3, 1, 0xFFFDU),
            // missing second byte
            std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1111, 0, 0}, 1, 0, 0xFFFDU),
            // missing third byte
            std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1111, 0b1011'1111, 0}, 2, 1, 0xFFFDU),
            // surrogate range
            std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1101, 0b1010'0000, 0b1000'0000}, 3, 0, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1101, 0b1011'0000, 0b1000'0000}, 3, 0, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1101, 0b1011'1111, 0b1011'1111}, 3, 0, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 3>{ 0b1110'1110, 0b1000'0000, 0b1000'0000}, 3, 0, 0xE000U)
        );
        
        auto& arr = std::get<0>(test);
        fstlog::byte_span_const input(arr.data(), std::get<1>(test));

        auto codep = fstlog::detail::utf::decode_utf8_char(input);

        CHECK(codep == std::get<3>(test));
        CHECK(input.size() == std::get<2>(test));
    }
    
    SECTION("four_byte") {
        // 0b1110'xxxx, 0b10yy'yyyy, 0b10zz'zzzz, 0b10aa'aaaa
        auto test = GENERATE(
            // valid encoded
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1001'0000, 0b1000'0000, 0b1000'0000}, 4, 0, 0x10000U),
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1011'1111, 0b1011'1111, 0b1011'1111}, 4, 0, 0x3FFFFU),
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0100, 0b1000'1111, 0b1011'1111, 0b1011'1111}, 4, 0, 0x10FFFFU),
            // exceeding max
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0101, 0b1000'0000, 0b1000'0000, 0b1000'0000}, 4, 0, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0111, 0b1011'1111, 0b1011'1111, 0b1011'1111}, 4, 0, 0xFFFDU),
            // overlong encoded
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1000'0000, 0b1000'0000, 0b1000'0000}, 4, 0, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1000'1111, 0b1011'1111, 0b1011'1111}, 4, 0, 0xFFFDU),
            // invalid first byte
            std::make_tuple(std::array<unsigned char, 4>{ 0b1011'0000, 0b1001'0000, 0b1000'0000, 0b1000'0000}, 4, 3, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 4>{ 0b1001'0000, 0b1011'1111, 0b1011'1111, 0b1011'1111}, 4, 3, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 4>{ 0b1010'0100, 0b1000'1111, 0b1011'1111, 0b1011'1111}, 4, 3, 0xFFFDU),
            // invalid second byte
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b0001'0000, 0b1000'0000, 0b1000'0000}, 4, 3, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1111'1111, 0b1011'1111, 0b1011'1111}, 4, 3, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0100, 0b0000'1111, 0b1011'1111, 0b1011'1111}, 4, 3, 0xFFFDU),
            // invalid third byte
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1001'0000, 0b0000'0000, 0b1000'0000}, 4, 2, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1011'1111, 0b1111'1111, 0b1011'1111}, 4, 2, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0100, 0b1000'1111, 0b0111'1111, 0b1011'1111}, 4, 2, 0xFFFDU),
            // invalid fourth byte
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1001'0000, 0b1000'0000, 0b0000'0000}, 4, 1, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1011'1111, 0b1011'1111, 0b1111'1111}, 4, 1, 0xFFFDU),
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0100, 0b1000'1111, 0b1011'1111, 0b0111'1111}, 4, 1, 0xFFFDU),
            // missing second byte
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0, 0, 0}, 1, 0, 0xFFFDU),
            // missing third byte
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1000'1111, 0, 0}, 2, 1, 0xFFFDU),
            // missing fourth byte
            std::make_tuple(std::array<unsigned char, 4>{ 0b1111'0000, 0b1000'1111, 0b1000'1111, 0}, 3, 2, 0xFFFDU)
        );

        auto& arr = std::get<0>(test);
        fstlog::byte_span_const input(arr.data(), std::get<1>(test));

        auto codep = fstlog::detail::utf::decode_utf8_char(input);

        CHECK(codep == std::get<3>(test));
        CHECK(input.size() == std::get<2>(test));
    }
};

TEST_CASE("safe_utf8_to_utf16") {
    std::array<char16_t, 128> buffer;
    char16_t* const buffer_begin = buffer.data();
    
    SECTION("not_enough_space") {
        SECTION("ascii") {
            buffer.fill('!');
            std::string_view str("ASCII\\string/str");
            fstlog::unaligned_span input(str.data(), str.size());
            fstlog::unaligned_span output(buffer.data(), 5);
            const auto result = fstlog::detail::utf::safe_utf8_to_utf16(input, output);
            CHECK(result.ec == fstlog::error_code::buff_full);
            CHECK(output.empty());
            CHECK(std::basic_string_view<char16_t>(buffer_begin, 5) == u"ASCII");
        }

        SECTION("multi_byte_utf8") {
            buffer.fill('!');
            std::string_view str("€€€");
            fstlog::byte_span_const input(reinterpret_cast<const unsigned char*>(str.data()), str.size());
            fstlog::unaligned_span output(buffer.data(), 1);
            auto result = fstlog::detail::utf::safe_utf8_to_utf16(input, output);
            CHECK(result.ec == fstlog::error_code::buff_full);
            CHECK(input.size() + 3 == str.size());
            CHECK(output.empty());
            CHECK(std::basic_string_view<char16_t>(buffer.data(), 1) == u"€");
        }
    }

    SECTION("invalid_utf8") {
        buffer.fill('!');
        std::array<unsigned char, 13> str{ 'I', 'n', 'v', 'a', 'l', 'i', 'd', ':', 128, 'c', 'h', 'a', 'r' };
        fstlog::byte_span_const input(str);
        fstlog::unaligned_span output(buffer);
        
        auto result = fstlog::detail::utf::safe_utf8_to_utf16(input, output);
        CHECK(result.ec == fstlog::error_code::input_bad);
        CHECK(input.size() == 4);
        CHECK(buffer.size() - output.size() == 8);
        CHECK(std::basic_string_view<char16_t>(buffer.data(), 8) == u"Invalid:");
    }

    SECTION("unsafe_string") {
        buffer.fill('!');
        std::array<unsigned char, 15> str{ 'U', 'n', 's', 'a', 'f', 'e',':', 0xf0, 0x9f, 0x98, 0x80, 'c', 'h', 'a', 'r' };
        fstlog::byte_span_const input(str);
        fstlog::unaligned_span output(buffer);

        auto result = fstlog::detail::utf::safe_utf8_to_utf16(input, output);
        CHECK(result.ec == fstlog::error_code::input_bad);
        CHECK(input.size() == 4);
        CHECK(buffer.size() - output.size() == 7);
        CHECK(std::basic_string_view<char16_t>(buffer.data(), 7) == u"Unsafe:");
    }

    SECTION("safe_string") {
        buffer.fill('!');
        std::string_view str("Safe utf8 string €€€ 编码者.");
        fstlog::byte_span_const input(reinterpret_cast<const unsigned char*>(str.data()), str.size());
        fstlog::unaligned_span output(buffer);

        auto result = fstlog::detail::utf::safe_utf8_to_utf16(input, output);
        CHECK(result.ec == fstlog::error_code::none);
        CHECK(input.empty());
        CHECK(buffer.size() - output.size() == 25);
        CHECK(std::basic_string_view<char16_t>(buffer.data(), 25) == u"Safe utf8 string €€€ 编码者.");
    }
}

TEST_CASE("safe_code_point_benchmark", "[.][benchmark]") {
    auto input = GENERATE(
        U"UTF encoding is a standard for encoding characters in the Unicode character set.",
        U"Kodowanie UTF jest standardem kodowania znaków w zestawie znaków Unicode.",
        U"Кодировка UTF является стандартом для кодирования символов в наборе символов Unicode.",
        U"UTF 编码者, Unicode 字符集字符编码之准也。"
    );

    DYNAMIC_SECTION("input_") {
        auto const begin = input;
        BENCHMARK_ADVANCED("safe_code_point")(Catch::Benchmark::Chronometer meter) {
            meter.measure([begin] {
                std::size_t res = 0;
                auto pos = begin;
                while (*pos != 0) {
                    res += fstlog::detail::utf::safe_utf_code_point(*pos++);
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
            std::make_tuple(std::string_view(""), 0, 0, 0),
            std::make_tuple(std::string_view(""), 10, 0, 0),
            std::make_tuple(std::string_view("a"), 0, 0, 0),
            std::make_tuple(std::string_view("a"), 1, 1, 1),
            std::make_tuple(std::string_view("a"), 10, 1, 1),
            std::make_tuple(std::string_view("UTF string"), 10, 10, 10),
            std::make_tuple(std::string_view("UTF string"), 10, 10, 10),
            std::make_tuple(std::string_view("UTF string"), 11, 10, 10),
            std::make_tuple(std::string_view("UTF 编码者"), 0, 0, 0),
            std::make_tuple(std::string_view("UTF 编码者"), 1, 1, 1),
            std::make_tuple(std::string_view("UTF 编码者"), 5, 5, 7),
            std::make_tuple(std::string_view("UTF 编码者"), 7, 7, 13),
            std::make_tuple(std::string_view("UTF 编码者"), 100, 7, 13),
            // unsafe data
            std::make_tuple(std::string_view("UTF \uFFFD码者"), 5, 5, 7),
            std::make_tuple(std::string_view("UTF \uFFFD码者"), 7, 7, 13),
            std::make_tuple(std::string_view("UTF 😀码者"), 100, 7, 14)
        );
        auto str = std::get<0>(data);
        fstlog::byte_span_const input(reinterpret_cast<const unsigned char*>(str.data()), str.size());
        std::size_t wanted_char_num = std::get<1>(data);
        std::size_t control_char_num = std::get<2>(data);
        std::size_t control_byte_num = std::get<3>(data);
        const auto [result_bytes, result_chars] = fstlog::detail::utf::utf8_str_trim(
            input,
            wanted_char_num);
        CHECK(result_chars == control_char_num);
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
        auto str = std::get<0>(data);
        fstlog::byte_span_const input(str);
        std::size_t wanted_char_num = std::get<1>(data);
        std::size_t control_char_num = std::get<2>(data);
        std::size_t control_byte_num = std::get<3>(data);
        const auto [result_bytes, result_chars] = fstlog::detail::utf::utf8_str_trim(
            input,
            wanted_char_num);
        CHECK(result_chars == control_char_num);
        CHECK(result_bytes == control_byte_num);
    }
}
