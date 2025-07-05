//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <detail/mixin/allocator_mixin.hpp>
#include <formatter/impl/logfield_formspec_txt_fast_mixin.hpp>

using test_type = typename
	fstlog::logfield_formspec_txt_fast_mixin<
	fstlog::allocator_mixin>;

TEST_CASE("logfield_formspec_txt_fast_mixin") {
	test_type instance;
	
	SECTION("01") {
		auto data = GENERATE(
			std::make_tuple(std::string_view("\xea\xb0\x80>+#30.12x"), fstlog::format_setting_txt_fast{ 'x', 0, 12}),
			std::make_tuple(std::string_view("\xea\xb0\x80>+#30.12"), fstlog::format_setting_txt_fast{ 0, 0, 12 }),
			std::make_tuple(std::string_view("\xea\xb0\x80>+#30.12."), fstlog::format_setting_txt_fast{ 0, 0, 0 }),
			std::make_tuple(std::string_view("\xea\xb0\x80>+#30.12.."), fstlog::format_setting_txt_fast{ 0, 0, 0 }),
			std::make_tuple(std::string_view("\xea\xb0\x80>+#30x"), fstlog::format_setting_txt_fast{ 'x', 0, 0xffff }),
			std::make_tuple(std::string_view("\xea\xb0\x80>"), fstlog::format_setting_txt_fast{ 0, 0, 0xffff }),
			std::make_tuple(std::string_view("30x"), fstlog::format_setting_txt_fast{ 'x', 0, 0xffff }),
			std::make_tuple(std::string_view(".12A"), fstlog::format_setting_txt_fast{ 'A', 0, 12 }),
			std::make_tuple(std::string_view("x"), fstlog::format_setting_txt_fast{ 'x', 0, 0xffff }),
			std::make_tuple(std::string_view(""), fstlog::format_setting_txt_fast{ 0, 0, 0xffff }),
			std::make_tuple(std::string_view("30"), fstlog::format_setting_txt_fast{ 0, 0, 0xffff }),
			std::make_tuple(std::string_view(".12345678"), fstlog::format_setting_txt_fast{0, 0, 1234}),
			std::make_tuple(std::string_view(".0001233456x"), fstlog::format_setting_txt_fast{ 'x', 0, 1233 }),
			std::make_tuple(std::string_view(".<30.0001233456x"), fstlog::format_setting_txt_fast{ 'x', 0, 1233 }),
			std::make_tuple(std::string_view(".<30"), fstlog::format_setting_txt_fast{ 0, 0, 0xffff })
		);
		fstlog::buff_span_const form_spec(
			reinterpret_cast<const unsigned char*>(std::get<0>(data).data()), std::get<0>(data).size());
		fstlog::format_setting_txt_fast control = std::get<1>(data);

		CHECK(instance.get_format(form_spec) == control);


	};
}
