//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once

namespace fstlog {
	template<auto thread_, class L>
	class logger_thread_compile_time_mixin : public L {
	public:
		static constexpr auto thread() noexcept {
			return thread_;
		}
	};
}
