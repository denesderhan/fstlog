//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>
#include <cstring>
#pragma intrinsic(memcmp)

namespace fstlog {
    struct alignas(4) format_setting_txt_fast {
        unsigned char type{ 0 };
        unsigned char padding{ 0 };
        std::uint16_t precision{ 0xffff };
    };

    inline bool operator==(
		format_setting_txt_fast lhs, 
		format_setting_txt_fast rhs) noexcept 
	{
        return 0 == memcmp(&lhs, &rhs, sizeof(format_setting_txt_fast));
    }
    inline bool operator!=(
		const format_setting_txt_fast lhs, 
		format_setting_txt_fast rhs) noexcept
	{ 
        return !operator==(lhs, rhs); 
    }

#ifdef FSTLOG_DEBUG
	static_assert(sizeof(format_setting_txt_fast) ==
		sizeof(format_setting_txt_fast::type)
		+ sizeof(format_setting_txt_fast::padding)
		+ sizeof(format_setting_txt_fast::precision));
#endif
}
