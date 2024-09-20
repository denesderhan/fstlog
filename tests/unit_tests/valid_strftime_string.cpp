//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

 
#include <detail/safe_reinterpret_cast.hpp>
#include <formatter/impl/detail/valid_strftime_string.hpp>

TEST_CASE("valid_strftime_string") {
	SECTION("valid") {
		std::string_view str = GENERATE(
			"",
			"!",
			"!!",
			"!!!!",
			"%Y",
			"%Od",
			"%%%%",
			"%%%%%%",
			"%%!%%%%!!",
			"!%S%m",
			"!!%EY",
			"!%Od",
			"Y%Y%%YOE",
			"%S%z%Z",
			"%z",
			"%Z");

		CAPTURE(str);
		fstlog::buff_span_const buff_sp(
			fstlog::safe_reinterpret_cast<const unsigned char*>(str.data()), 
			str.size());

		CHECK(fstlog::detail::valid_strftime_string(buff_sp, fstlog::tz_format::Local));
	};

	SECTION("invalid") {
		std::string_view str = GENERATE(
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
			"%z",
			"%Z",
			"%S%z%Z",
			"%H:%M:%S %z %Z",
			"H%:%M:%S %z");
		
		CAPTURE(str);
		fstlog::buff_span_const buff_sp(fstlog::safe_reinterpret_cast<const unsigned char*>(str.data()), str.size());

		CHECK(!fstlog::detail::valid_strftime_string(buff_sp, fstlog::tz_format::UTC));
	};
}
