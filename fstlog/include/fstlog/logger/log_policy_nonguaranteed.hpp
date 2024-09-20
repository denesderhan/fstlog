//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>
#include <utility>

#include <fstlog/detail/level.hpp>
#include <fstlog/detail/log_policy.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
	template<class L>
	class log_policy_nonguaranteed : public L {
	public:
		template<level level, log_call_flag flags, typename... Args>
		void log(Args const&... args) noexcept(
			noexcept(this->write_pos())
			&& noexcept(this->can_write())
			&& noexcept(this->template write_message<level, log_policy::NonGuaranteed, flags>(args...))
			&& noexcept(this->request_flush_if_needed(std::declval<decltype(L::write_pos())&>())))
		{
			std::uint32_t w_pos_begin = L::write_pos();
			if (can_write()) {
				L::template write_message<level, log_policy::NonGuaranteed, flags>(args...);
				L::request_flush_if_needed(w_pos_begin);
			}
		}
		template<log_call_flag flags, typename... Args>
		void log(level level, Args const&... args) noexcept(
			noexcept(this->write_pos())
			&& noexcept(this->can_write())
			&& noexcept(this->template write_message<log_policy::NonGuaranteed, flags>(level, args...))
			&& noexcept(this->request_flush_if_needed(std::declval<decltype(L::write_pos())&>())))
		{
			std::uint32_t w_pos_begin = L::write_pos();
			if (can_write()) {
				L::template write_message<log_policy::NonGuaranteed, flags>(level, args...);
				L::request_flush_if_needed(w_pos_begin);
			}
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
