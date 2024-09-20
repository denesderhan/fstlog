//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/logger/detail/this_thread_id.hpp>

namespace fstlog {
	template<class L>
	class logger_thread_query_mixin : public L {
	public:
		static auto thread() noexcept {
			return this_thread::get_id();
		}
	};
}
