//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/logger/detail/this_thread_id.hpp>
#include <fstlog/detail/small_string.hpp>
#include <fstlog/detail/fast_to_str.hpp>

namespace fstlog {
	namespace this_thread {
		inline small_string<32> get_str() noexcept {
			const auto thr_id{ this_thread::get_id() };
			char buffer[24]{ '0' };
			return to_dec(thr_id, buffer);
		}
	}
}
