//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

 
#include <detail/safe_reinterpret_cast.hpp>
#include <formatter/impl/detail/valid_strftime_string.hpp>
TEST_CASE("valid_strft_conv_spec") {
    for (int i = 0; i < 256; i++) {
        unsigned char conv_spec = static_cast<unsigned char>(i);
        auto pos = "HMSYadmyz";
        while (*pos != 0 && *pos != conv_spec) pos++;
        if (*pos == 0) {
            CHECK(fstlog::detail::valid_strft_conv_spec(conv_spec) == false);
        }
        else {
            CHECK(fstlog::detail::valid_strft_conv_spec(conv_spec) == true);
        }
    }
};

TEST_CASE("valid_strftime_string") {
    SECTION("valid") {
        std::string_view str = GENERATE(
            "!",
            "!!",
            "!!!!",
            "%Y",
            "%%%%",
            "%%%%%%",
            "%%!%%%%!!",
            "!%S%m",
            "%S%z",
            "%z",
            "Y%Y%%YOE",
            "%H:%M:%S %z");

        CAPTURE(str);
        fstlog::byte_span_const buff_sp(
            fstlog::safe_reinterpret_cast<const unsigned char*>(str.data()), 
            str.size());

        CHECK(fstlog::detail::valid_strftime_string(buff_sp));
    };

    SECTION("invalid") {
        std::string_view str = GENERATE(
            "",
            "%Od",
            "!!%EY",
            "!%Od",
            "%",
            "%%%",
            "%%%%%",
            "x%",
            "XX%",
            "xxx%",
            "%!",
            "%O!",
            "%O",
            "%O!!",
            "%E",
            "%E!!!",
            "%Z",
            "%S%z%Z",
            "%H:%M:%S %z %Z",
            "H%:%M:%S %z",
            "H%:%M:%S %S");
        
        CAPTURE(str);
        fstlog::byte_span_const buff_sp(fstlog::safe_reinterpret_cast<const unsigned char*>(str.data()), str.size());

        CHECK(!fstlog::detail::valid_strftime_string(buff_sp));
    };
}
