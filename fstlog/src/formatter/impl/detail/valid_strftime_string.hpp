//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstdint>

#include <detail/byte_span.hpp>
#include <formatter/impl/detail/tz_format.hpp>
#include <detail/utf8_helper.hpp>

namespace fstlog {
	namespace detail{
		// validate a limited set of conversion specifiers
		inline constexpr bool valid_strft_conv_spec(unsigned char spec) noexcept {
			// LUT for chars 'A' - 'z' range in a single 64 bit uint
			constexpr std::uint64_t valid_conv_spec_lut =
				  (1ULL << ('A' - 'A')) | (1ULL << ('a' - 'A'))
				| (1ULL << ('B' - 'A')) | (1ULL << ('b' - 'A'))
				| (1ULL << ('C' - 'A')) 
				| (1ULL << ('d' - 'A'))	| (1ULL << ('e' - 'A'))
				| (1ULL << ('G' - 'A')) | (1ULL << ('g' - 'A'))
				| (1ULL << ('H' - 'A')) | (1ULL << ('I' - 'A'))
				| (1ULL << ('j' - 'A')) 
				| (1ULL << ('M' - 'A')) | (1ULL << ('m' - 'A'))
				| (1ULL << ('p' - 'A')) | (1ULL << ('S' - 'A'))
				| (1ULL << ('U' - 'A')) | (1ULL << ('u' - 'A'))
				| (1ULL << ('V' - 'A'))
				| (1ULL << ('W' - 'A')) | (1ULL << ('w' - 'A'))
				| (1ULL << ('Y' - 'A')) | (1ULL << ('y' - 'A'))
				| (1ULL << ('Z' - 'A')) | (1ULL << ('z' - 'A'));

			// bounds check
			if (spec < 'A' || spec > 'z') return false;

			// calculate the bit position
			const auto bit_pos = spec - 'A';

			// shift the LUT bit to the least significant place and check if it is set.
			return (valid_conv_spec_lut >> bit_pos) & 1;
		}

		// Validate time string for strftime call,
		// with a limited set of conversion specifiers
		// for maximum compatibility.
		inline constexpr bool valid_strftime_string(
			buff_span_const str, 
			tz_format tz) noexcept 
		{
			const std::size_t str_len = str.size_bytes();
			if (str_len == 0) return false;

			const auto str_end{ str.data() + str_len };
			std::size_t pos = 0;
			std::size_t num_sec_specifiers{ 0 };
			while (pos < str_len) {
				const auto bytes{ valid_utf8(&str[pos], str_end) };
				if (bytes == 0) {
					return false; //invalid utf8
				}
				if (!printable_utf8(&str[pos], bytes)) {
					return false; // invalid code point or control char
				}
				// start of a conversion specifier
				if (str[pos] == '%') {
					if (pos + 1 == str_len) {
						return false; // invalid '%' at end
					}
					unsigned char conv_spec = str[pos + 1];
					if (conv_spec != '%' && !valid_strft_conv_spec(conv_spec)) {
						return false; // invalid conversion specifier (%% is valid)
					}
					if (tz == tz_format::UTC && (conv_spec == 'z' || conv_spec == 'Z')) {
						return false; // allow formatting local zone only when time is in local
					}
					if (conv_spec == 'S') {
						num_sec_specifiers++; // count number of '%S' (only 1 is allowed)
					}
					pos += 2; // skip the valid  '%' + conversion specifier
				}
				else {
					pos += bytes; // skip the utf8 char
				}
			}
			return num_sec_specifiers <= 1;
		}
	}
}
