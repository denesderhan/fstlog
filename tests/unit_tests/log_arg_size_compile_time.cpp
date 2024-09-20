//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <fstlog/logger/detail/log_arg_size_compile_time.hpp>

TEST_CASE("log_arg_size_compile_time") {
	SECTION("01") {
		CHECK(fstlog::log_arg_size_compile_time<char>::value == true);
		CHECK(fstlog::log_arg_size_compile_time<char16_t>::value == true);
		CHECK(fstlog::log_arg_size_compile_time<bool>::value == true);
		CHECK(fstlog::log_arg_size_compile_time<int>::value == true);
		CHECK(fstlog::log_arg_size_compile_time<float>::value == true);
		CHECK(fstlog::log_arg_size_compile_time<char*>::value == true);
		CHECK(fstlog::log_arg_size_compile_time<fstlog::str_hash_fnv>::value == true);
		
		CHECK(fstlog::log_arg_size_compile_time<decltype("Hi")>::value == true);
		CHECK(fstlog::log_arg_size_compile_time<std::array<char, 4>>::value == true);
		CHECK(fstlog::log_arg_size_compile_time<std::pair<char, int>>::value == true);
		CHECK(fstlog::log_arg_size_compile_time<
			std::tuple<std::pair<int, double>, int, std::tuple<float, std::array<char, 3>>>
		>::value == true);
		
		CHECK(fstlog::log_arg_size_compile_time<std::string_view>::value == false);
		CHECK(fstlog::log_arg_size_compile_time<std::string>::value == false);
		CHECK(fstlog::log_arg_size_compile_time<std::vector<int>>::value == false);

		CHECK(fstlog::log_arg_size_compile_time<
			std::tuple<std::pair<int, double>, int, std::tuple<std::string, int>>
		>::value == false);
		CHECK(fstlog::log_arg_size_compile_time<
			std::tuple<std::pair<int, double>, std::vector<std::string>, std::tuple<std::string, int>>
		>::value == false);
	};
}
