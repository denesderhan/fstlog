//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <detail/mixin/memory_resource_mixin.hpp>
#include <formatter/impl/logfield_formspec_txt_mixin.hpp>

using test_type = typename
    fstlog::logfield_formspec_txt_mixin<
    fstlog::memory_resource_mixin>;

TEST_CASE("logfield_formspec_txt_mixin") {
    test_type instance(fstlog::get_default_resource());
    SECTION("01") {
        fstlog::format_setting_txt form{};
        CHECK(test_type::get_default_format() == form);
        std::string_view form_spec{"\xea\xb0\x80>+#30.12x"};
        fstlog::byte_span_const f(reinterpret_cast<const unsigned char*>(form_spec.data()), form_spec.size());
        
        form.type = 'x';
        form.alternate = true;
        form.align = '>';
        form.fill_char = { 0xea, 0xb0, 0x80, 0x00 };
        form.sign = '+';
        form.width = 30;
        form.precision = 12;

        for (unsigned char i = 0; i < fstlog::ut_cast(fstlog::logfield_last); i++) {
            instance.set_format(fstlog::logfield{ i }, f);
            CHECK( instance.get_format(fstlog::logfield{ i }) == form);
        }
        
    };

    SECTION("02") {
        auto data = GENERATE(
            std::make_tuple(std::string_view("\xea\xb0\x80>+#30.12x"), fstlog::format_setting_txt{ 'x', true, 30, 12, '+', '>', {0xea, 0xb0, 0x80, 0x00} }),
            std::make_tuple(std::string_view(".<-#30.12x"), fstlog::format_setting_txt{ 'x', true, 30, 12, '-', '<', {'.', 0,0,0}}),
            std::make_tuple(std::string_view(">> 88.12b"), fstlog::format_setting_txt{ 'b', false, 88, 12, ' ', '>', {'>',0,0,0}}),
            std::make_tuple(std::string_view("+#30.12."), fstlog::format_setting_txt{ '.', true, 30, 12, '+', 0, {' ',0,0,0}}),
            std::make_tuple(std::string_view("30.12.."), fstlog::format_setting_txt{ '.', false, 30, 12, '-', 0, {' ',0,0,0}}),
            std::make_tuple(std::string_view("a^+#33"), fstlog::format_setting_txt{ 0, true, 33, 0xffff, '+', '^', {'a',0,0,0}}),
            std::make_tuple(std::string_view("d>"), fstlog::format_setting_txt{ 0, false, 0, 0xffff, '-', 0, {' ',0,0,0}}),
            std::make_tuple(std::string_view("30x"), fstlog::format_setting_txt{ 'x', false, 30, 0xffff, '-', 0, {' ',0,0,0}}),
            std::make_tuple(std::string_view(".12A"), fstlog::format_setting_txt{ 'A', false, 0, 12, '-', 0, {' ',0,0,0}}),
            std::make_tuple(std::string_view("x"), fstlog::format_setting_txt{ 'x', false, 0, 0xffff, '-', 0, {' ',0,0,0}}),
            std::make_tuple(std::string_view(""), fstlog::format_setting_txt{ 0, false, 0, 0xffff, '-', 0, {' ',0,0,0}}),
            std::make_tuple(std::string_view("30"), fstlog::format_setting_txt{ 0, false, 30, 0xffff, '-', 0, {' ',0,0,0}}),
            std::make_tuple(std::string_view(".12345678"), fstlog::format_setting_txt{ 0, false, 0, 1234, '-', 0, {' ',0,0,0}}),
            std::make_tuple(std::string_view(".0001233456x"), fstlog::format_setting_txt{ 'x', false, 0, 1233, '-', 0, {' ',0,0,0}})
        );
        std::string_view form_spec = std::get<0>(data);
        CAPTURE(form_spec);
        fstlog::byte_span_const form(
            reinterpret_cast<const unsigned char*>(form_spec.data()), form_spec.size());
        fstlog::format_setting_txt control = std::get<1>(data);
        auto res = instance.get_format(form);
        CHECK(res == control);
    };
}
