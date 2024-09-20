//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/small_string.hpp>
#include <fstlog/logger/detail/this_thread_str.hpp>

namespace fstlog {
	template<class L>
	class logger_thread_thread_local_mixin : public L {
	public:
		static small_string<32> thread() noexcept {
			return thr_name_;
		}

		static void set_thread(small_string<32> name) noexcept {
			thr_name_ = name;
		}

		thread_local inline static small_string<32> thr_name_{fstlog::this_thread::get_str()};
	};
}
