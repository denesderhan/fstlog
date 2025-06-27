//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstdint>

#include <detail/byte_span.hpp>
#include <formatter/impl/detail/tz_format.hpp>
#include <detail/utf_conv.hpp>

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
			if (str.empty()) return false;
			const unsigned char *pos = str.data();
			const auto end{ pos + str.size_bytes() };
			std::size_t num_sec_specifiers{ 0 };
			while (pos < end) {
				auto code_point = decode_utf8_char(pos, end);
				if (!safe_utf_code_point(code_point)) return false;
				
				// start of a conversion specifier
				if (code_point == '%') {
					if (pos == end) {
						return false; // invalid '%' at end
					}
					// consume conversion specifier
					unsigned char conv_spec = *pos++;
					if (conv_spec != '%' && !valid_strft_conv_spec(conv_spec)) {
						return false; // invalid conversion specifier (%% is valid)
					}
					if (tz == tz_format::UTC && (conv_spec == 'z' || conv_spec == 'Z')) {
						return false; // allow formatting local zone only when time is in local
					}
					if (conv_spec == 'S') {
						num_sec_specifiers++; // count number of '%S' (only 1 is allowed)
					}
				}
			}
			return num_sec_specifiers <= 1;
		}
	}
}
