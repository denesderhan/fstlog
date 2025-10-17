//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>
 
#include <fstlog/detail/small_string.hpp>

#include <string.h>

#include <fstlog/detail/log_type.hpp>

static fstlog::small_string<32> convert_small_string(fstlog::small_string<24> in) {
    return in;
}

static constexpr bool test_constexpr_mutation_small_string()
{
    fstlog::small_string<64> s; // Create a local object
    s.push_back('A');           // Mutate it
    s.push_back('B');           // Mutate it again
    s.pop_back();               // And again
    s += "CD";                  // And again

    // Now check the final state
    return s.size() == 3 && s[0] == 'A' && s[1] == 'C' && s[2] == 'D';
}

static_assert(test_constexpr_mutation_small_string(), "This will fail to compile in C++17");

TEST_CASE("small_string") {
    
    SECTION("construct") {
        
        CHECK(fstlog::log_type_v<fstlog::small_string<16>> == fstlog::log_element_type::Smallstring);

        CHECK(sizeof(fstlog::small_string<16>) == 16);
        fstlog::small_string<16> sstr0 = "Hello World!";
        CHECK(sstr0 == std::string_view{ "Hello World!" });
        CHECK(strcmp("Hello World!", sstr0.data()) == 0);
        CHECK(fstlog::small_string<16>{""} == std::string_view{ "" });
        CHECK(strcmp("", fstlog::small_string<16>{""}.data()) == 0);
        CHECK(fstlog::small_string<128>().size() == 0);
        CHECK(fstlog::small_string<64>().empty());
        
        fstlog::small_string<16> sstr1 = "15 chr long str";
        CHECK(!sstr1.empty());
        CHECK(sstr1.size() == 15);
        CHECK(sstr1 == "15 chr long str");
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
        CHECK(fstlog::small_string<40>{ "" } == fstlog::small_string<40>{ "" });
        CHECK(fstlog::small_string<44>{ "" } != fstlog::small_string<44>{ "0" });
        CHECK(fstlog::small_string<32>{ "123" } == fstlog::small_string<32>{ "123" });
        
        char test[5]{ 'T', 'e', 'x', 't', 0 };
        fstlog::small_string<16> sstr4{test};
        CHECK(std::string_view{ sstr4 } == "Text");
    };

    SECTION("assign_add") {
        fstlog::small_string<16> sstr0{ "Hello!" };
        constexpr fstlog::small_string<16> sstr1{ "Good by!" };
        auto sstr2 = sstr0 + sstr1;
        CHECK(std::string_view{ sstr2 } == "Hello!Good by!");
        sstr2 = sstr1;
        CHECK(std::string_view{ sstr2 } == "Good by!");
        sstr0 += sstr0;
        CHECK(std::string_view{ sstr0 } == "Hello!Hello!");

        sstr0 += std::string_view{ "1" };
        sstr0 = sstr0 + "2345";
        CHECK(std::string_view{ sstr0 } == "Hello!Hello!123");

        fstlog::small_string<8> sstr3{ "ABCDEF" };
        auto strv = convert_small_string(sstr3);
        CHECK(strv == "ABCDEF");
        fstlog::small_string<128> sstr4;
        sstr4 = sstr3;
        sstr4 += 'G';
        sstr4 = sstr4 + sstr4;
        CHECK(sstr4 == "ABCDEFGABCDEFG");
    };

    SECTION("compare") {
        fstlog::small_string<16> sstr0{ "" };
        fstlog::small_string<16> sstr1{ "a" };
        fstlog::small_string<16> sstr2{ "aa" };
        fstlog::small_string<16> sstr3{ "bb" };
        fstlog::small_string<16> sstr4{ "b" };
        CHECK(sstr0 < sstr1);
        CHECK(sstr1 < sstr2);
        CHECK(sstr2 < sstr3);
        CHECK(sstr4 < sstr3);
        CHECK(sstr2 < sstr4);
        CHECK(sstr4 > sstr0);
        CHECK(sstr4 > sstr2);

        fstlog::small_string<8> sstr5{ "" };
        fstlog::small_string<8> sstr6{ "a" };
        fstlog::small_string<8> sstr7{ "aa" };
        fstlog::small_string<8> sstr8{ "bb" };
        fstlog::small_string<8> sstr9{ "b" };
        CHECK(sstr0 == sstr5);
        CHECK(sstr3 == sstr8);
        CHECK(sstr0 < sstr6);
        CHECK(sstr2 < sstr9);
        CHECK(sstr9 > sstr2);
        CHECK(sstr4 > sstr7);
    };

    SECTION("methods") {
        constexpr fstlog::small_string<64> str{ "TEXT" };
        constexpr bool begin_check = str.data() == str.begin();
        CHECK(begin_check);
        constexpr bool end_check = str.end() == str.data() + str.size();
        CHECK(end_check);
        constexpr bool cbegin_check = str.c_str() == str.cbegin();
        CHECK(cbegin_check);
        constexpr bool cend_check = str.cend() == str.c_str() + str.size();
        CHECK(cend_check);
        CHECK(str[0] == 'T');
        constexpr bool is_empty = str.empty();
        CHECK(is_empty == false);
        constexpr bool mut = test_constexpr_mutation_small_string();
        CHECK(mut == true);
    };
}
