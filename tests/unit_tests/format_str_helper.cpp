//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <vector>

#include <formatter/impl/detail/format_str_helper.hpp>

TEST_CASE("format_str_helper") {
	SECTION("parse_fmt_text") {
		std::vector<unsigned char> out_buff(10);
		const unsigned char* out_end = out_buff.data() + out_buff.size();
		SECTION("error_code::fmt_bad") {
			
			std::tuple<std::string_view, std::string_view, int> test_dat = GENERATE(
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "}" }, std::string_view{ "" }, 0 },
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ ":}" }, std::string_view{ ":" }, 1 },
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "}XXX" }, std::string_view{ "" }, 0},
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "ab:}}}" }, std::string_view{ "ab:}" }, 5},
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "{{ab:}" }, std::string_view{ "{ab:" }, 5},
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "abc}" }, std::string_view{ "abc" }, 3},
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "abc}xyz" }, std::string_view{ "abc" }, 3},
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "\1bc}xyz" }, std::string_view{ "_bc" }, 3},
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "\1\2\3abxyz}}{{}" }, std::string_view{ "___abxyz}{" }, 12}
			);
			auto in_beg = reinterpret_cast<const unsigned char*>(std::get<0>(test_dat).data());
			auto in_pos = in_beg;
			auto in_end = in_beg + std::get<0>(test_dat).size();
			auto out_pos = out_buff.data();

			auto error = fstlog::parse_fmt_text(in_pos, in_end, out_pos, out_end);
			CHECK(error == fstlog::error_code::fmt_bad);
			CHECK(in_pos <= in_end);
			CHECK(out_pos <= out_end);
			std::string_view repl_out{ 
				reinterpret_cast<const char*>(out_buff.data()),
				static_cast<std::size_t>(out_pos - out_buff.data()) };
			CHECK(repl_out == std::get<1>(test_dat));
			CHECK(in_pos == in_beg + std::get<2>(test_dat));
		};

		SECTION("error_code::buff_full") {

			std::tuple<std::string_view, std::string_view, int, int> test_dat = GENERATE(
				std::tuple<std::string_view, std::string_view, int, int>{std::string_view{ "12345678901}" }, std::string_view{ "1234567890" }, 10, 10 },
				std::tuple<std::string_view, std::string_view, int, int>{std::string_view{ "}}2345{{789}}{{" }, std::string_view{ "}2345{789}" }, 13, 10},
				std::tuple<std::string_view, std::string_view, int, int>{std::string_view{ "}}2345{{789}}{{" }, std::string_view{ "" }, 0, 0},
				std::tuple<std::string_view, std::string_view, int, int>{std::string_view{ "a" }, std::string_view{ "" }, 0, 0}
			);
			auto in_beg = reinterpret_cast<const unsigned char*>(std::get<0>(test_dat).data());
			auto in_pos = in_beg;
			auto in_end = in_beg + std::get<0>(test_dat).size();
			out_buff.resize(std::get<3>(test_dat));
			auto out_pos = out_buff.data();
			out_end = out_pos + out_buff.size();
			
			auto error = fstlog::parse_fmt_text(in_pos, in_end, out_pos, out_end);
			CHECK(error == fstlog::error_code::buff_full);
			CHECK(in_pos <= in_end);
			CHECK(out_pos <= out_end);
			std::string_view repl_out{
				reinterpret_cast<const char*>(out_buff.data()),
				static_cast<std::size_t>(out_pos - out_buff.data()) };
			CHECK(repl_out == std::get<1>(test_dat));
			CHECK(in_pos == in_beg + std::get<2>(test_dat));
		};

		out_buff.resize(512);
		out_end = out_buff.data() + out_buff.size();

		SECTION("error_code::none") {

			std::tuple<std::string_view, std::string_view, int> test_dat = GENERATE(
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "{" }, std::string_view{ "" }, 0 },
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ ":{" }, std::string_view{ ":" }, 1 },
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "{XXX" }, std::string_view{ "" }, 0},
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "ab:}}{" }, std::string_view{ "ab:}" }, 5},
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "{{ab:{" }, std::string_view{ "{ab:" }, 5},
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "abc{" }, std::string_view{ "abc" }, 3},
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "abc{xyz" }, std::string_view{ "abc" }, 3},
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "\1bc{xyz" }, std::string_view{ "_bc" }, 3},
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "\1\2\3abxyz}}{{{" }, std::string_view{ "___abxyz}{" }, 12},
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "" }, std::string_view{ "" }, 0 },
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "{{{{}}}}" }, std::string_view{ "{{}}" }, 8 },
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "abcd" }, std::string_view{ "abcd" }, 4 },
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "{name:}" }, std::string_view{ "" }, 0 },
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "abc{name:}" }, std::string_view{ "abc" }, 3 },
				std::tuple<std::string_view, std::string_view, int>{std::string_view{ "abc\1\2:}}{{{abc" }, std::string_view{ "abc__:}{" }, 10 }
			);
			auto in_beg = reinterpret_cast<const unsigned char*>(std::get<0>(test_dat).data());
			auto in_pos = in_beg;
			auto in_end = in_beg + std::get<0>(test_dat).size();
			auto out_pos = out_buff.data();

			auto error = fstlog::parse_fmt_text(in_pos, in_end, out_pos, out_end);
			CHECK(error == fstlog::error_code::none);
			CHECK(in_pos <= in_end);
			CHECK(out_pos <= out_end);
			std::string_view repl_out{
				reinterpret_cast<const char*>(out_buff.data()),
				static_cast<std::size_t>(out_pos - out_buff.data()) };
			CHECK(repl_out == std::get<1>(test_dat));
			CHECK(in_pos == in_beg + std::get<2>(test_dat));
		};
	};

	SECTION("parse_fmt_repl_field") {
		
		SECTION("error_code::fmt_bad") {
			std::tuple<std::string_view, std::string_view, std::string_view, int> test_dat = GENERATE(
				std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abcd{xyz" }, std::string_view{ "" }, std::string_view{ "" }, 5 },
				std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abcd{{xyz" }, std::string_view{ "" }, std::string_view{ "" }, 5 },
				std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abcd:{xyz" }, std::string_view{ "" }, std::string_view{ "" }, 6 },
				std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{ab:d{xyz" }, std::string_view{ "" }, std::string_view{ "" }, 5 },
				std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abcd" }, std::string_view{ "" }, std::string_view{ "" }, 5 },
				std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abcd:xyz" }, std::string_view{ "" }, std::string_view{ "" }, 9 }
			);
			auto in_beg = reinterpret_cast<const unsigned char*>(std::get<0>(test_dat).data());
			auto in_pos = in_beg;
			auto in_end = in_beg + std::get<0>(test_dat).size();
			
			fstlog::buff_span_const field_name;
			fstlog::buff_span_const format_spec;
			auto error = fstlog::parse_fmt_repl_field(in_pos, in_end, field_name, format_spec);
			std::string_view name{
				reinterpret_cast<const char*>(field_name.data()),
				static_cast<std::size_t>(field_name.size_bytes()) };
			std::string_view spec{
				reinterpret_cast<const char*>(format_spec.data()),
				static_cast<std::size_t>(format_spec.size_bytes()) };
			CHECK(error == fstlog::error_code::fmt_bad);
			CHECK(in_pos <= in_end);
			CHECK(name == std::get<1>(test_dat));
			CHECK(spec == std::get<2>(test_dat));
			CHECK(in_pos == in_beg + std::get<3>(test_dat));
		};

		SECTION("error_code::none") {
			std::tuple<std::string_view, std::string_view, std::string_view, int> test_dat = GENERATE(
				std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abcd}{xyz" }, std::string_view{ "abcd" }, std::string_view{ "" }, 6 },
				std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abcd:}{xyz" }, std::string_view{ "abcd" }, std::string_view{ "" }, 7 },
				std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{ab:d}{xyz" }, std::string_view{ "ab" }, std::string_view{ "d" }, 6 },
				std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abc}d" }, std::string_view{ "abc" }, std::string_view{ "" }, 5 },
				std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abcd:xyz}" }, std::string_view{ "abcd" }, std::string_view{ "xyz" }, 10 },
				std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{:}" }, std::string_view{ "" }, std::string_view{ "" }, 3 },
				std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{}" }, std::string_view{ "" }, std::string_view{ "" }, 2 },
				std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{::}}" }, std::string_view{ "" }, std::string_view{ ":" }, 4 },
				std::tuple<std::string_view, std::string_view, std::string_view, int>{std::string_view{ "{abcd:xy:z}" }, std::string_view{ "abcd" }, std::string_view{ "xy:z" }, 11 }
			);
			auto in_beg = reinterpret_cast<const unsigned char*>(std::get<0>(test_dat).data());
			auto in_pos = in_beg;
			auto in_end = in_beg + std::get<0>(test_dat).size();
			
			fstlog::buff_span_const field_name;
			fstlog::buff_span_const format_spec;
			auto error = fstlog::parse_fmt_repl_field(in_pos, in_end, field_name, format_spec);
			std::string_view name{
				reinterpret_cast<const char*>(field_name.data()),
				static_cast<std::size_t>(field_name.size_bytes()) };
			std::string_view spec{
				reinterpret_cast<const char*>(format_spec.data()),
				static_cast<std::size_t>(format_spec.size_bytes()) };
			CHECK(error == fstlog::error_code::none);
			CHECK(in_pos <= in_end);
			CHECK(name == std::get<1>(test_dat));
			CHECK(spec == std::get<2>(test_dat));
			CHECK(in_pos == in_beg + std::get<3>(test_dat));
		};
	};

	SECTION("field_name_id") {
		for (int ind = 0; ind < fstlog::ut_cast(fstlog::logfield_last); ind++) {
			fstlog::logfield field = fstlog::logfield(ind);
			auto name = fstlog::get_repl_field_name(field);
			fstlog::buff_span_const temp(reinterpret_cast<const unsigned char*>(name.data()), name.length());
			CHECK(field == fstlog::get_repl_field_id(temp));
		}
		CHECK(fstlog::logfield_last == fstlog::logfield::Args);
		CHECK(fstlog::get_repl_field_name(fstlog::logfield::Args) == "invalid");
	};
}
