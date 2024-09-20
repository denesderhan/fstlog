//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/small_string.hpp>

namespace fstlog {
	template<class L>
	class logger_thread_mixin : public L {
	public:
		logger_thread_mixin() noexcept = default;
		~logger_thread_mixin() noexcept = default;

		logger_thread_mixin(const logger_thread_mixin& other) noexcept
			: L(other),
			thr_name_{ "Unnamed" } {}

		logger_thread_mixin(logger_thread_mixin&& other) noexcept
			: L(std::move(other)),
			thr_name_{ other.thr_name_ }
		{
			other.thr_name_ = "Unnamed";
		}

		logger_thread_mixin& operator=(const logger_thread_mixin& other) noexcept {
			L::operator=(other);
			thr_name_ = "Unnamed";
			return *this;
		}

		logger_thread_mixin& operator=(logger_thread_mixin&& other) noexcept {
			L::operator=(std::move(other));
			thr_name_ = other.thr_name_;
			other.thr_name_ = "Unnamed";
			return *this;
		}

		small_string<32> thread() const noexcept {
			return thr_name_;
		}

		void set_thread(small_string<32> name) noexcept {
			thr_name_ = name;
		}

		small_string<32> thr_name_{ "Unnamed"};
	};
}
