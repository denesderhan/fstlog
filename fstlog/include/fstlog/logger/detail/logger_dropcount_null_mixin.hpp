//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once

namespace fstlog {
	template<class L>
	class logger_dropcount_null_mixin : public L {
	public:
		static void count_dropped() noexcept {}
	};
}
