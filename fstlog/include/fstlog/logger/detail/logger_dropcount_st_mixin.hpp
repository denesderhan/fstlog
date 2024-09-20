//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>

namespace fstlog {
	template<class L>
	class logger_dropcount_st_mixin : public L {
	public:
		logger_dropcount_st_mixin() noexcept = default;

		~logger_dropcount_st_mixin() noexcept = default;

		logger_dropcount_st_mixin(const logger_dropcount_st_mixin& other) noexcept
			: L(other),
			dropped_{ 0 } {}

		logger_dropcount_st_mixin(logger_dropcount_st_mixin&& other) noexcept
			: L(std::move(other)),
			dropped_{ other.dropped_ }
		{
			other.dropped_ = 0;
		}

		logger_dropcount_st_mixin& operator=(const logger_dropcount_st_mixin& other) noexcept {
			L::operator=(other);
			dropped_ = 0;
			return *this;
		}

		logger_dropcount_st_mixin& operator=(logger_dropcount_st_mixin&& other) noexcept {
			L::operator=(std::move(other));
			dropped_ = other.dropped_;
			other.dropped_ = 0;
			return *this;
		}
		
		std::uintmax_t dropped() const noexcept {
			return dropped_;
		}

		void count_dropped() noexcept {
			dropped_++;
		}

		std::uintmax_t dropped_{0};
	};
}
