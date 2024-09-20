//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/small_string.hpp>

namespace fstlog {
	template<class L>
	class logger_name_mixin : public L {
	public:
		logger_name_mixin() noexcept = default;
		~logger_name_mixin() noexcept = default;

		logger_name_mixin(const logger_name_mixin& other) noexcept
			: L(other),
			logger_name_{ "Unnamed" } {}

		logger_name_mixin(logger_name_mixin&& other) noexcept
			: L(std::move(other)),
			logger_name_{ other.logger_name_ }
		{
			other.logger_name_ = "Unnamed";
		}

		logger_name_mixin& operator=(const logger_name_mixin& other) noexcept {
			L::operator=(other);
			logger_name_ = "Unnamed";
			return *this;
		}

		logger_name_mixin& operator=(logger_name_mixin&& other) noexcept {
			L::operator=(std::move(other));
			logger_name_ = other.logger_name_;
			other.logger_name_ = "Unnamed";
			return *this;
		}

		small_string<32> name() const noexcept {
			return logger_name_;
		}

		void set_name(small_string<32> name) noexcept {
			logger_name_ = name;
		}

		small_string<32> logger_name_{ "Unnamed" };
	};
}
