//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>
#include <detail/error_code.hpp>

namespace fstlog {
	inline error_code uint_fromchars_4digit(
		std::uint16_t& num,
		const unsigned char*& begin, 
		const unsigned char* end) noexcept
	{
		static_assert('9' < 127 && '0' < '9' && '0' + 1 == '1'
			&& '0' + 2 == '2' && '0' + 3 == '3' && '0' + 4 == '4'
			&& '0' + 5 == '5' && '0' + 6 == '6' && '0' + 7 == '7'
			&& '0' + 8 == '8' && '0' + 9 == '9');
		auto end_ptr = begin;
		while (end_ptr < end && *end_ptr <= '9' && *end_ptr >= '0') {
			end_ptr++;
		}
		const auto char_num{ end_ptr - begin };
		if (char_num == 1) {
			num = *begin++ - '0';
			return error_code::none;
		}
		else if (char_num != 0 
			&& char_num <= 4
			&& *begin != '0') 
		{
			auto pos = end_ptr - 1;
			std::uint16_t out{ 0 };
			std::uint16_t mult{ 1 };
			while (true) {
				//can not overflow, max 4 digits
				out += mult * (*pos - '0');
				if (pos == begin) break;
				pos--;
				mult *= 10;
			}
			begin = end_ptr;
			num = out;
			return error_code::none;
        }
		else return error_code::input_contract_violation;
    }
}
