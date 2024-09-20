//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>
#include <cstring>
#pragma intrinsic(memcmp)

namespace fstlog {
    struct alignas(4) format_setting_txt {
        unsigned char type{ 0 };
        bool alternate{ false };
		std::uint16_t width{ 0 };
		std::uint16_t precision{ 0xffff };
        unsigned char sign{ '-' };
        unsigned char align{ 0 };
        unsigned char fill_char[4]{' ', 0, 0, 0 };
    };

    inline bool operator==(
		format_setting_txt lhs, 
		format_setting_txt rhs) noexcept 
	{
        return 0 == memcmp(&lhs, &rhs, sizeof(format_setting_txt));
    }
    inline bool operator!=(
		const format_setting_txt lhs, 
		format_setting_txt rhs) noexcept 
	{ 
        return !operator==(lhs, rhs); 
    }

#ifdef FSTLOG_DEBUG
	static_assert(sizeof(format_setting_txt) == 
		sizeof(format_setting_txt::type)
		+ sizeof(format_setting_txt::alternate)
		+ sizeof(format_setting_txt::width)
		+ sizeof(format_setting_txt::precision)
		+ sizeof(format_setting_txt::sign)
		+ sizeof(format_setting_txt::align)
		+ sizeof(format_setting_txt::fill_char));
#endif
}
