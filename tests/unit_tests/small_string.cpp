//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>
 
#include <fstlog/detail/small_string.hpp>

#include <string.h>

#include <fstlog/detail/log_type.hpp>

TEST_CASE("small_string") {
	
	SECTION("01") {
		
		CHECK(fstlog::log_type_v<fstlog::small_string<16>> == fstlog::log_element_type::Smallstring);

		CHECK(sizeof(fstlog::small_string<16>) == 16);
		fstlog::small_string<16> sstr0 = "Hello World!";
		CHECK(sstr0 == std::string_view{ "Hello World!" });
		CHECK(strcmp("Hello World!", sstr0.data()) == 0);
		CHECK(fstlog::small_string<16>{""} == std::string_view{ "" });
		CHECK(strcmp("", fstlog::small_string<16>{""}.data()) == 0);
		CHECK(fstlog::small_string<256>().size() == 0);
		CHECK(fstlog::small_string<64>().empty());
		
		fstlog::small_string<16> sstr1 = "15 chr long str";
		CHECK(!sstr1.empty());
		CHECK(sstr1.size() == 15);
		CHECK(sstr1 == std::string_view{ "15 chr long str" });
		CHECK(strcmp("15 chr long str", sstr1.data()) == 0);

		fstlog::small_string<16> sstr2 = "16 char long str";
		CHECK(!sstr2.empty());
		CHECK(sstr2.size() == 15);
		CHECK(sstr2 == std::string_view{ "16 char long st" });
		CHECK(strcmp("16 char long st", sstr2.data()) == 0);
		CHECK(sstr1 != sstr2);
		
		sstr2 = "Short";
		CHECK(sstr2 == std::string_view{ "Short" });
		CHECK(strcmp("Short", sstr2.data()) == 0);

		const char* text = "Hello World!";
		fstlog::small_string<16> sstr3(text);
		CHECK(strcmp("Hello World!", sstr3.data()) == 0);

		CHECK(sstr1 == fstlog::small_string<16>{ "15 chr long str this will be cut off" });
		CHECK(fstlog::small_string<16>{ "15 chr long str" } == fstlog::small_string<16>{ "15 chr long str this will be cut off" });
		CHECK(fstlog::small_string<33>{ "" } == fstlog::small_string<33>{ "" });
		CHECK(fstlog::small_string<33>{ "" } != fstlog::small_string<33>{ "0" });
		CHECK(fstlog::small_string<32>{ "123" } == fstlog::small_string<32>{ "123" });
		
		char test[4]{'T', 'e', 'x', 't'};
		char test2[10]{ 'T', 'e', 'x', 't', 0, 'e', 'r', 'r', 'o', 'r'};
		char test3[5]{ 'T', 'e', 'x', 't', 0 };
		fstlog::small_string<16> sst1{test3};
		fstlog::small_string<16> sst3{test2};
		fstlog::small_string<16> sst2{"Text"};
		CHECK(sst1 == sst3);
		CHECK(sst1 == sst2);
	};
}
