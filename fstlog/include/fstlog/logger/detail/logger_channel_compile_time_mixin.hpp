//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/types.hpp>

namespace fstlog {
	template<channel_type channel_, class L>
	class logger_channel_compile_time_mixin : public L {
	public:
		static constexpr channel_type channel() noexcept {
			return channel_;
		}
	};
}
