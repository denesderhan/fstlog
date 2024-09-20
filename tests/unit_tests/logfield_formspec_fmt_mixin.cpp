//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <detail/mixin/allocator_mixin.hpp>
#include <formatter/impl/logfield_formspec_fmt_mixin.hpp>

using test_type = typename
	fstlog::logfield_formspec_fmt_mixin<
	fstlog::allocator_mixin>;

TEST_CASE("logfield_formspec_fmt_mixin") {
	test_type instance;
	SECTION("01") {
		CHECK(test_type::get_default_format() == fstlog::small_string<24>("{}"));
		std::string_view form_spec{"?>+#30.12Lx"};
		fstlog::buff_span_const f(reinterpret_cast<const unsigned char*>(form_spec.data()), form_spec.size());
		instance.set_format(fstlog::logfield::Channel, f);
		CHECK(std::string_view{instance.get_format(fstlog::logfield::Channel)} == "{:?>+#30.12Lx}");
		CHECK(std::string_view{ instance.get_format_align_width("")} == "{}");
		CHECK(std::string_view{ instance.get_format_align_width("{:}") } == "{}");
		CHECK(std::string_view{ instance.get_format_align_width("{:?>+#30.12Lx}") } == "{:?>30}");
		CHECK(std::string_view{ instance.get_format_align_width("{:>+#30.12Lx}") } == "{:>30}");
		CHECK(std::string_view{ instance.get_format_align_width("{:+#30.12Lx}") } == "{:30}");
		CHECK(std::string_view{ instance.get_format_align_width("{:?>+#.12Lx}") } == "{:?>}");
		CHECK(std::string_view{ instance.get_format_align_width("{:>+#30}") } == "{:>30}");
		CHECK(std::string_view{ instance.get_format_align_width("{:?>}") } == "{:?>}");
		CHECK(std::string_view{ instance.get_format_align_width("{:>30.12Lx}") } == "{:>30}");
		CHECK(std::string_view{ instance.get_format_align_width("{:+#30.12Lx}") } == "{:30}");
		CHECK(std::string_view{ instance.get_format_align_width("{:30.12Lx}") } == "{:30}");
		CHECK(std::string_view{ instance.get_format_align_width("{:30}") } == "{:30}");
		CHECK(std::string_view{ instance.get_format_align_width("{:?>}") } == "{:?>}");
		CHECK(std::string_view{ instance.get_format_align_width("{:>}") } == "{:>}");
		CHECK(std::string_view{ instance.get_format_align_width("{:?>+#030.12Lx}") } == "{:?>30}");
		CHECK(std::string_view{ instance.get_format_align_width("{:?>+030.12Lx}") } == "{:?>30}");
		CHECK(std::string_view{ instance.get_format_align_width("{:?>#030.12Lx}") } == "{:?>30}");
		CHECK(std::string_view{ instance.get_format_align_width("{:?>#30.12Lx}") } == "{:?>30}");
		CHECK(std::string_view{ instance.get_format_align_width("{:?>030.12Lx}") } == "{:?>30}");
		CHECK(std::string_view{ instance.get_format_align_width("{:?>+30.12Lx}") } == "{:?>30}");
		CHECK(std::string_view{ instance.get_format_align_width("{:?>+#0.12Lx}") } == "{:?>}");
		CHECK(std::string_view{ instance.get_format_align_width("{:?>+0.12Lx}") } == "{:?>}");
		CHECK(std::string_view{ instance.get_format_align_width("{:?>#0.12Lx}") } == "{:?>}");
		CHECK(std::string_view{ instance.get_format_align_width("{:?>#.12Lx}") } == "{:?>}");
		CHECK(std::string_view{ instance.get_format_align_width("{:?>0.12Lx}") } == "{:?>}");
		CHECK(std::string_view{ instance.get_format_align_width("{:?>+.12Lx}") } == "{:?>}");
	};
}
