//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/level.hpp>
#include <fstlog/detail/log_policy.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
	template<class L>
	class log_policy_lowlatency : public L {
	public:
		template<level level, log_call_flag flags, typename... Args>
		void log(Args const&... args) noexcept(
			noexcept(this->can_write())
			&& noexcept(this->template write_message<level, log_policy::LowLatency, flags>(args...)))
		{
			if (can_write())
				L::template write_message<level, log_policy::LowLatency, flags>(args...);
		}
		template<log_call_flag flags, typename... Args>
		void log(level level, Args const&... args) noexcept(
			noexcept(this->can_write())
			&& noexcept(this->template write_message<log_policy::LowLatency, flags>(level, args...)))
		{
			if (can_write())
				L::template write_message<log_policy::LowLatency, flags>(level, args...);
		}
	private:
		bool can_write() noexcept(
			noexcept(this->message_fits())
			&& noexcept(this->update_buffer_state())
			&& noexcept(this->count_dropped()))
		{
			if (L::message_fits()) return true;
			L::update_buffer_state();
			if (L::message_fits()) return true;
			L::count_dropped();
			return false;
		}
	};
}
