//Copyright © Dénes Derhán 2025.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <array>
#include <cstring>

#include <detail/error.hpp>

TEST_CASE("fstlog_error") {
	std::array<unsigned char, 256> buffer{ 0 };
	
	SECTION("write_to") {
		
		// error, control, buffer size 
#ifdef FSTLOG_DEBUG
		auto test_dat = GENERATE(
			std::make_tuple(fstlog::error{}, std::string_view(""), 0),
			std::make_tuple(fstlog::error{}, std::string_view(""), 6),
			std::make_tuple(fstlog::error{}, std::string_view("... Message truncated, fstlog error: No error! ?:?"), 256),
			std::make_tuple(fstlog::error{}, std::string_view("... Message truncated, fstlog error: No error! ?:?"), 100),
			std::make_tuple(fstlog::error{}, std::string_view("... fstlog error:000"), 99),
			std::make_tuple(fstlog::error{}, std::string_view("... fstlog error:000"), 20),
			std::make_tuple(fstlog::error{}, std::string_view("Err:000"), 19),
			std::make_tuple(fstlog::error{}, std::string_view("Err:000"), 7),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code::input_bad }, std::string_view("... Message truncated, fstlog error: Input data was malformed or corrupted! /home/prog/my_path/my_project/my_file.cpp:001"), 256),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code::input_bad }, std::string_view("... Message truncated, fstlog error: Input data was malformed or corrupted!"), 100),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code::input_bad }, std::string_view("... fstlog error:001"), 99),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code::input_bad }, std::string_view("... fstlog error:001"), 20),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code::input_bad }, std::string_view("Err:001"), 19),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code::input_bad }, std::string_view("Err:001"), 7),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code(999) }, std::string_view("... Message truncated, fstlog error: Unknown error! /home/prog/my_path/my_project/my_file.cpp:001"), 100),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code(999) }, std::string_view("... fstlog error:999"), 99),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code(999) }, std::string_view("... fstlog error:999"), 20),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code(999) }, std::string_view("Err:999"), 19),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code(999) }, std::string_view("Err:999"), 7),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 10000, fstlog::error_code(1111) }, std::string_view("... Message truncated, fstlog error: Unknown error! /home/prog/my_path/my_project/my_file.cpp:?"), 100),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 10000, fstlog::error_code(1111) }, std::string_view("... fstlog error:?"), 99),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 10000, fstlog::error_code(1111) }, std::string_view("... fstlog error:?"), 20),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 10000, fstlog::error_code(1111) }, std::string_view("Err:?"), 19),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 10000, fstlog::error_code(1111) }, std::string_view("Err:?"), 7),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(2) }, std::string_view("... Message truncated, fstlog error: Not enough space in output buffer! path:042"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(3) }, std::string_view("... Message truncated, fstlog error: Recursion limit reached! path:042"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(4) }, std::string_view("... Message truncated, fstlog error: Error in external code! path:042"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(5) }, std::string_view("... Message truncated, fstlog error: Object is already initialized! path:042"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(6) }, std::string_view("... Message truncated, fstlog error: Memory allocation failed! path:042"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(7) }, std::string_view("... Message truncated, fstlog error: Thread execution failed! path:042"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(8) }, std::string_view("... Message truncated, fstlog error: Object does not exists! path:042"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(9) }, std::string_view("... Message truncated, fstlog error: Object is already used, access denied! path:042"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(10) }, std::string_view("... Message truncated, fstlog error: Missing C++ language feature! path:042"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(11) }, std::string_view("... Message truncated, fstlog error: String too long! path:042"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(12) }, std::string_view("... Message truncated, fstlog error: Invalid fmt format string! path:042"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(13) }, std::string_view("... Message truncated, fstlog error: Core not set! path:042"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(14) }, std::string_view("... Message truncated, fstlog error: Buffer not set! path:042"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(15) }, std::string_view("... Message truncated, fstlog error: Path invalid or too long! path:042"), 100)
		);
#else
		auto test_dat = GENERATE(
			std::make_tuple(fstlog::error{}, std::string_view(""), 0),
			std::make_tuple(fstlog::error{}, std::string_view(""), 6),
			std::make_tuple(fstlog::error{}, std::string_view("... Message truncated, fstlog error: No error!"), 256),
			std::make_tuple(fstlog::error{}, std::string_view("... Message truncated, fstlog error: No error!"), 100),
			std::make_tuple(fstlog::error{}, std::string_view("... fstlog error:000"), 99),
			std::make_tuple(fstlog::error{}, std::string_view("... fstlog error:000"), 20),
			std::make_tuple(fstlog::error{}, std::string_view("Err:000"), 19),
			std::make_tuple(fstlog::error{}, std::string_view("Err:000"), 7),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code::input_bad }, std::string_view("... Message truncated, fstlog error: Input data was malformed or corrupted!"), 256),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code::input_bad }, std::string_view("... Message truncated, fstlog error: Input data was malformed or corrupted!"), 100),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code::input_bad }, std::string_view("... fstlog error:001"), 99),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code::input_bad }, std::string_view("... fstlog error:001"), 20),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code::input_bad }, std::string_view("Err:001"), 19),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code::input_bad }, std::string_view("Err:001"), 7),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code(999) }, std::string_view("... Message truncated, fstlog error: Unknown error!"), 100),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code(999) }, std::string_view("... fstlog error:999"), 99),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code(999) }, std::string_view("... fstlog error:999"), 20),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code(999) }, std::string_view("Err:999"), 19),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 1, fstlog::error_code(999) }, std::string_view("Err:999"), 7),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 10000, fstlog::error_code(1111) }, std::string_view("... Message truncated, fstlog error: Unknown error!"), 100),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 10000, fstlog::error_code(1111) }, std::string_view("... fstlog error:?"), 99),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 10000, fstlog::error_code(1111) }, std::string_view("... fstlog error:?"), 20),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 10000, fstlog::error_code(1111) }, std::string_view("Err:?"), 19),
			std::make_tuple(fstlog::error{ "/home/prog/my_path/my_project/my_file.cpp", 10000, fstlog::error_code(1111) }, std::string_view("Err:?"), 7),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(2) }, std::string_view("... Message truncated, fstlog error: Not enough space in output buffer!"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(3) }, std::string_view("... Message truncated, fstlog error: Recursion limit reached!"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(4) }, std::string_view("... Message truncated, fstlog error: Error in external code!"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(5) }, std::string_view("... Message truncated, fstlog error: Object is already initialized!"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(6) }, std::string_view("... Message truncated, fstlog error: Memory allocation failed!"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(7) }, std::string_view("... Message truncated, fstlog error: Thread execution failed!"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(8) }, std::string_view("... Message truncated, fstlog error: Object does not exists!"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(9) }, std::string_view("... Message truncated, fstlog error: Object is already used, access denied!"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(10) }, std::string_view("... Message truncated, fstlog error: Missing C++ language feature!"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(11) }, std::string_view("... Message truncated, fstlog error: String too long!"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(12) }, std::string_view("... Message truncated, fstlog error: Invalid fmt format string!"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(13) }, std::string_view("... Message truncated, fstlog error: Core not set!"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(14) }, std::string_view("... Message truncated, fstlog error: Buffer not set!"), 100),
			std::make_tuple(fstlog::error{ "path", 42, fstlog::error_code(15) }, std::string_view("... Message truncated, fstlog error: Path invalid or too long!"), 100)
		);
#endif
		auto err = std::get<0>(test_dat);
		auto buff_end = buffer.data() + std::get<2>(test_dat);
		
		auto str_end = err.write_to(buffer.data(), buff_end);
		auto contrl_str = std::get<1>(test_dat);
		CAPTURE(contrl_str);
		CHECK(static_cast<std::size_t>(str_end - buffer.data()) == contrl_str.size());
		CHECK(!memcmp(contrl_str.data(), buffer.data(), str_end - buffer.data()));
	};

}
