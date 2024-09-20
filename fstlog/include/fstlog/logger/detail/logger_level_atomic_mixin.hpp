//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <atomic> 

#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/level.hpp>

namespace fstlog {
	template<class L>
	class logger_level_atomic_mixin : public L {
	public:
		logger_level_atomic_mixin() noexcept = default;

		~logger_level_atomic_mixin() noexcept = default;
		
		logger_level_atomic_mixin(const logger_level_atomic_mixin& other) noexcept
			: L(other),
			level_{other.level_.load(std::memory_order_relaxed)} {}

		logger_level_atomic_mixin(logger_level_atomic_mixin&& other) noexcept
			: L(std::move(other)), 
			level_{other.level_.load(std::memory_order_relaxed)} {}

		logger_level_atomic_mixin& operator=(const logger_level_atomic_mixin& other) noexcept {
			L::operator=(other);
			level_.store(other.level_.load(std::memory_order_relaxed), std::memory_order_relaxed);
			return *this;
		}

		logger_level_atomic_mixin& operator=(logger_level_atomic_mixin&& other) noexcept {
			L::operator=(std::move(other));
			level_.store(other.level_.load(std::memory_order_relaxed), std::memory_order_relaxed);
			return *this;
		}
		
		fstlog::level level() const noexcept {
			return level_.load(std::memory_order_relaxed);
		}

		void set_level(fstlog::level level) noexcept {
			level_.store(level, std::memory_order_relaxed);
		}

		std::atomic<fstlog::level> level_{ fstlog::level::All};
#ifdef FSTLOG_DEBUG
		static_assert(decltype(level_)::is_always_lock_free);
#endif
	};
}
