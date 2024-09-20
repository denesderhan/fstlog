//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <atomic>
#include <cstdint>

namespace fstlog {
	template<class L>
	class logger_dropcount_mixin : public L {
	public:
		logger_dropcount_mixin() noexcept = default;
		~logger_dropcount_mixin() noexcept = default;

		logger_dropcount_mixin(const logger_dropcount_mixin& other) noexcept 
			: L(other),
			dropped_{ 0 } {}
		logger_dropcount_mixin(logger_dropcount_mixin&& other) noexcept 
			: L(std::move(other)),
			dropped_{other.dropped_.exchange(0, std::memory_order_relaxed)} {}
		logger_dropcount_mixin& operator=(const logger_dropcount_mixin& other) noexcept {
			L::operator=(other);
			dropped_.store(0, std::memory_order_relaxed);
			return *this;
		}
		logger_dropcount_mixin& operator=(logger_dropcount_mixin&& other) noexcept {
			L::operator=(std::move(other));
			dropped_.store(other.dropped_.exchange(0, std::memory_order_relaxed), std::memory_order_relaxed);
			return *this;
		}
						
		std::uintmax_t dropped() const noexcept {
			return dropped_.load(std::memory_order_relaxed);
		}

		void count_dropped() noexcept {
			dropped_.fetch_add(1, std::memory_order_relaxed);
		}

		std::atomic<std::uintmax_t> dropped_{0};
#ifdef FSTLOG_DEBUG
		static_assert(decltype(dropped_)::is_always_lock_free, "Type not lock_free!");
#endif
	};
}
