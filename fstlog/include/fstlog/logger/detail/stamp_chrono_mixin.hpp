//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <chrono>
#include <fstlog/detail/types.hpp>

namespace fstlog {
	template<class L>
	class stamp_chrono_mixin : public L {
	public:
		static stamp_type timestamp() noexcept {
			return std::chrono::system_clock::now();
		}
	};
}
