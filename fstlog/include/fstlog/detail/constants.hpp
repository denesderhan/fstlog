//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>

#include <fstlog/detail/types.hpp>
#include <fstlog/detail/is_pow2.hpp>

namespace fstlog {
	namespace constants {
		inline constexpr std::size_t cache_ls_nosharing{ 64 };

		inline constexpr std::size_t internal_msg_alignment{ 8 };
		inline constexpr std::size_t internal_msg_data_alignment{ 4 };
		
		inline constexpr channel_type self_log_channel{ 0 };
		inline constexpr channel_type default_log_channel{ 1 };

#ifdef FSTLOG_DEBUG
		static_assert(internal_msg_data_alignment <= internal_msg_alignment
			&& (internal_msg_alignment % internal_msg_data_alignment) == 0,
			"fstlog, internal_msg_data_alignment invalid value!");
		static_assert(is_pow2(internal_msg_alignment),
			"fstlog, internal_msg_alignment must be power of two!");
#endif
	}
}
