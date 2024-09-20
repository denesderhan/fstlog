//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/level.hpp>

namespace fstlog {
	template<fstlog::level level_, class L>
	class logger_level_compile_time_mixin : public L {
	public:
		static constexpr fstlog::level level() noexcept {
			return level_;
		}
	};
}
