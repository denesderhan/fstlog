//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/level.hpp>

namespace fstlog {
	template<class L>
	class logger_level_mixin : public L {
	public:
		fstlog::level level() const noexcept {
			return level_;
		}

		void set_level(fstlog::level level) noexcept {
			level_ = level;
		}

		fstlog::level level_{level::All};
	};
}
