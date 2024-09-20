//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <formatter/impl/detail/format_str_helper.hpp>

TEST_CASE("format_str_helper") {
	SECTION("repl_field") {
		SECTION("empty") {
			std::string_view repl_in = GENERATE(std::string_view{ "{}" }, std::string_view{ "{}XXX" });
				
			fstlog::buff_span_const r_in(reinterpret_cast<const unsigned char*>(repl_in.data()), repl_in.size());
			auto r_out = fstlog::get_replacement_field(r_in);
			std::string_view repl_out{ reinterpret_cast<const char*>(r_out.data()), r_out.size_bytes() };
			CAPTURE(repl_in);
			CHECK(repl_out == "");
		};

		SECTION("{:}") {
			std::string_view repl_in = GENERATE(std::string_view{ "{:}" }, std::string_view{ "{:}XXX" });

			fstlog::buff_span_const r_in(reinterpret_cast<const unsigned char*>(repl_in.data()), repl_in.size());
			auto r_out = fstlog::get_replacement_field(r_in);
			std::string_view repl_out{ reinterpret_cast<const char*>(r_out.data()), r_out.size_bytes() };
			CAPTURE(repl_in);
			CHECK(repl_out == ":");
		};
		SECTION("{abc:def}") {
			std::string_view repl_in = GENERATE(std::string_view{ "{abc:def}" }, std::string_view{ "{abc:def}XXX" });

			fstlog::buff_span_const r_in(reinterpret_cast<const unsigned char*>(repl_in.data()), repl_in.size());
			auto r_out = fstlog::get_replacement_field(r_in);
			std::string_view repl_out{ reinterpret_cast<const char*>(r_out.data()), r_out.size_bytes() };
			CAPTURE(repl_in);
			CHECK(repl_out == "abc:def");
		};
	};

	SECTION("format_spec") {
		SECTION("empty") {
			std::string_view repl_in = GENERATE(std::string_view{ "" }, std::string_view{ "name" });

			fstlog::buff_span_const r_in(reinterpret_cast<const unsigned char*>(repl_in.data()), repl_in.size());
			auto f_out = fstlog::format_spec(r_in);
			std::string_view form_out{ reinterpret_cast<const char*>(f_out.data()), f_out.size_bytes() };
			CAPTURE(repl_in);
			CHECK(form_out == "");
		};

		SECTION("{:}") {
			std::string_view repl_in = GENERATE(std::string_view{ ":" }, std::string_view{ "name:" });

			fstlog::buff_span_const r_in(reinterpret_cast<const unsigned char*>(repl_in.data()), repl_in.size());
			auto f_out = fstlog::format_spec(r_in);
			std::string_view form_out{ reinterpret_cast<const char*>(f_out.data()), f_out.size_bytes() };
			CAPTURE(repl_in);
			CHECK(form_out == "");
		};

		SECTION("{:abc}") {
			std::string_view repl_in = GENERATE(std::string_view{ ":abc" }, std::string_view{ "name:abc" });

			fstlog::buff_span_const r_in(reinterpret_cast<const unsigned char*>(repl_in.data()), repl_in.size());
			auto f_out = fstlog::format_spec(r_in);
			std::string_view form_out{ reinterpret_cast<const char*>(f_out.data()), f_out.size_bytes() };
			CAPTURE(repl_in);
			CHECK(form_out == "abc");
		};
	};

	SECTION("valid_format_string") {
		SECTION("tests") {
			auto extent = GENERATE(table<std::string_view, bool>({
					std::tuple<std::string_view, bool>{"", false},
					std::tuple<std::string_view, bool>{"{", false},
					std::tuple<std::string_view, bool>{"}", false},
					std::tuple<std::string_view, bool>{"{{{", false},
					std::tuple<std::string_view, bool>{"{{}", false},
					std::tuple<std::string_view, bool>{"}{", false},
					std::tuple<std::string_view, bool>{"{{{}", true},
					std::tuple<std::string_view, bool>{"{{}", false},
					std::tuple<std::string_view, bool>{"abcd{", false},
					std::tuple<std::string_view, bool>{"abcd{}}", false},
					std::tuple<std::string_view, bool>{"abcd{ef{}gh}ijkl", false},
					std::tuple<std::string_view, bool>{"abcd{ef{{g}}h}ijkl", false},
					std::tuple<std::string_view, bool>{"abcd{{efgh}}ij{kl}mn", true},
					std::tuple<std::string_view, bool>{"a", true},
					std::tuple<std::string_view, bool>{"a{bc}def{{", true},
					std::tuple<std::string_view, bool>{"ab}}cd{ef}{}gh}}", true},
					std::tuple<std::string_view, bool>{"Name: {name:}<-#20.4d}", false},
					std::tuple<std::string_view, bool>{"{severity} {time:*^30.6L%H:%T %Z} {line:d} {{{message:<}}}", true},
					std::tuple<std::string_view, bool>{"\0", false},
					std::tuple<std::string_view, bool>{"\1", false},
					std::tuple<std::string_view, bool>{"\xc2\x80", true}
				}));

			std::string_view string = std::get<0>(extent);
			//bool result = std::get<1>(extent);

			CAPTURE(string);
			fstlog::buff_span_const in(reinterpret_cast<const unsigned char*>(string.data()), string.size());
			//CHECK(result == fstlog::valid_format_string(in));
		}
	};
}
