//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once

namespace fstlog {
	template<auto name_, class L>
	class logger_name_compile_time_mixin : public L {
	public:
		static constexpr auto name() noexcept {
			return name_;
		}
	};
}
