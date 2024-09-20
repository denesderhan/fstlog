//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <cstddef>
#include <limits>

#include <detail/nearest_pow2.hpp>


TEST_CASE("nearest_pow2_01") {
	SECTION("size_t") {
		auto extent = GENERATE(table<size_t, size_t>({
			std::tuple<std::size_t, std::size_t>{std::size_t(0), std::size_t(1)},
			std::tuple<std::size_t, std::size_t>{std::size_t(1), std::size_t(1)},
			std::tuple<std::size_t, std::size_t>{std::size_t(2), std::size_t(2)},
			std::tuple<std::size_t, std::size_t>{std::size_t(3), std::size_t(4)},
			std::tuple<std::size_t, std::size_t>{std::size_t(4), std::size_t(4)},
			std::tuple<std::size_t, std::size_t>{(std::numeric_limits<std::size_t>::max)(), std::size_t(1) << ((sizeof(std::size_t) * std::numeric_limits<unsigned char>::digits) - 1)},
			std::tuple<std::size_t, std::size_t>{std::size_t(1024), std::size_t(1024)},
			std::tuple<std::size_t, std::size_t>{std::size_t(1500), std::size_t(1024)},
			std::tuple<std::size_t, std::size_t>{std::size_t(27 * 1024), std::size_t(32 * 1024)},
			std::tuple<std::size_t, std::size_t>{std::size_t(1) << ((sizeof(std::size_t) * std::numeric_limits<unsigned char>::digits) - 1), std::size_t(1) << ((sizeof(std::size_t) * std::numeric_limits<unsigned char>::digits) - 1) }
			}));

		std::size_t num = std::get<0>(extent);
		std::size_t expected = std::get<1>(extent);

		CAPTURE(num, expected);
		CHECK(fstlog::detail::nearest_pow2(num) == expected);

	};

	SECTION("uint32") {
		auto extent = GENERATE(table<std::uint32_t, std::uint32_t>({
			std::tuple<std::uint32_t, std::uint32_t>{std::uint32_t(0), std::uint32_t(1)},
			std::tuple<std::uint32_t, std::uint32_t>{std::uint32_t(1), std::uint32_t(1)},
			std::tuple<std::uint32_t, std::uint32_t>{std::uint32_t(2), std::uint32_t(2)},
			std::tuple<std::uint32_t, std::uint32_t>{std::uint32_t(3), std::uint32_t(4)},
			std::tuple<std::uint32_t, std::uint32_t>{std::uint32_t(4), std::uint32_t(4)},
			std::tuple<std::uint32_t, std::uint32_t>{(std::numeric_limits<std::uint32_t>::max)(), std::uint32_t(1) << ((sizeof(std::uint32_t) * std::numeric_limits<unsigned char>::digits) - 1)},
			std::tuple<std::uint32_t, std::uint32_t>{std::uint32_t(1024), std::uint32_t(1024)},
			std::tuple<std::uint32_t, std::uint32_t>{std::uint32_t(1500), std::uint32_t(1024)},
			std::tuple<std::uint32_t, std::uint32_t>{std::uint32_t(27 * 1024), std::uint32_t(32 * 1024)},
			std::tuple<std::uint32_t, std::uint32_t>{std::uint32_t(1) << ((sizeof(std::uint32_t) * std::numeric_limits<unsigned char>::digits) - 1), std::uint32_t(1) << ((sizeof(std::uint32_t) * std::numeric_limits<unsigned char>::digits) - 1) }
			}));

		std::uint32_t num = std::get<0>(extent);
		std::uint32_t expected = std::get<1>(extent);

		CAPTURE(num, expected);
		CHECK(fstlog::detail::nearest_pow2(num) == expected);

	};

	SECTION("uint16") {
		auto extent = GENERATE(table<std::uint16_t, std::uint16_t>({
			std::tuple<std::uint16_t, std::uint16_t>{std::uint16_t(0), std::uint16_t(1)},
			std::tuple<std::uint16_t, std::uint16_t>{std::uint16_t(1), std::uint16_t(1)},
			std::tuple<std::uint16_t, std::uint16_t>{std::uint16_t(2), std::uint16_t(2)},
			std::tuple<std::uint16_t, std::uint16_t>{std::uint16_t(3), std::uint16_t(4)},
			std::tuple<std::uint16_t, std::uint16_t>{std::uint16_t(4), std::uint16_t(4)},
			std::tuple<std::uint16_t, std::uint16_t>{(std::numeric_limits<std::uint16_t>::max)(), std::uint16_t(std::uint16_t(1) << ((sizeof(std::uint16_t) * std::numeric_limits<unsigned char>::digits) - 1))},
			std::tuple<std::uint16_t, std::uint16_t>{std::uint16_t(1024), std::uint16_t(1024)},
			std::tuple<std::uint16_t, std::uint16_t>{std::uint16_t(1500), std::uint16_t(1024)},
			std::tuple<std::uint16_t, std::uint16_t>{std::uint16_t(27 * 1024), std::uint16_t(32 * 1024)},
			std::tuple<std::uint16_t, std::uint16_t>{std::uint16_t(std::uint16_t(1) << ((sizeof(std::uint16_t) * std::numeric_limits<unsigned char>::digits) - 1)), std::uint16_t(std::uint16_t(1) << ((sizeof(std::uint16_t) * std::numeric_limits<unsigned char>::digits) - 1)) }
			}));

		std::uint16_t num = std::get<0>(extent);
		std::uint16_t expected = std::get<1>(extent);

		CAPTURE(num, expected);
		CHECK(fstlog::detail::nearest_pow2(num) == expected);

	};
}
