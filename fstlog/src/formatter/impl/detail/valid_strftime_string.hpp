//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstdint>

#include <detail/unaligned_span.hpp>
#include <formatter/impl/detail/tz_format.hpp>
#include <detail/utf_conv.hpp>

namespace fstlog {
	namespace detail{
		// validate a limited set of strftime conversion specifiers
		inline constexpr bool valid_strft_conv_spec(unsigned char spec) noexcept {
			constexpr std::uint64_t valid_conv_spec_lut =
				//    hour 00-23            minute 00-59       second 00-60 
				(1ULL << ('H' - 64)) | (1ULL << ('M' - 64))	| (1ULL << ('S' - 64)) 
				//    year 2025             day abbreviated      day of month
				| (1ULL << ('Y' - 64)) | (1ULL << ('a' - 64)) | (1ULL << ('d' - 64))
				//     month 01-12          year 00-99          utc offset +0130
				| (1ULL << ('m' - 64)) | (1ULL << ('y' - 64)) | (1ULL << ('z' - 64));

			// bounds check
			if (spec < 64 || spec > 127) return false;

			// calculate the bit position
			const auto bit_pos = spec - 64;

			// shift the LUT bit to the least significant place and check if it is set.
			return (valid_conv_spec_lut >> bit_pos) & 1;
		}

		// Validate time string for strftime call,
		// with a limited set of conversion specifiers
		// for maximum compatibility.
		inline constexpr bool valid_strftime_string(
			byte_span_const str,
			tz_format tz) noexcept
		{
			if (str.empty()) return false;
            bool has_second{ false };
			while (!str.empty()) {
				std::uint32_t code_point = detail::utf::decode_utf8_char(str);
				if (!utf::safe_utf_code_point(code_point)) return false;
				
				// start of a conversion specifier
				if (code_point == '%') {
					if (str.empty()) {
						return false; // invalid '%' at end
					}
					// consume conversion specifier
					unsigned char conv_spec = str.template get<0>();
                    str.template drop_front<1>();
					if (conv_spec != '%' && !valid_strft_conv_spec(conv_spec)) {
						return false; // invalid conversion specifier (%% is valid)
					}
					if (tz == tz_format::UTC && conv_spec == 'z') {
						return false; // if time is in UTC usage of %z is forbidden
					}
					if (conv_spec == 'S') {
                        if (has_second) {
                            return false; // only 1 %S allowed (cached time with second placeholder)
                        }
                        else {
                            has_second = true;
                        }
					}
				}
			}
			return true;
		}
	}
}
