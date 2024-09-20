//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/level.hpp>
#include <fstlog/detail/log_metaargs.hpp>
#include <fstlog/detail/types.hpp>
#include <fstlog/detail/ut_cast.hpp>

namespace fstlog {
	template<class L>
	class log_addmeta_mixin : public L {
	public:
		template<
			level level,
			template<class T> class policy,
			log_call_flag flags,
			typename... Args>
		void log(Args const&... args) noexcept(
			noexcept(L{}.template log<level, policy, flags>(args...))
			&& noexcept(this->name())
			&& noexcept(this->thread()))
		{
			constexpr std::size_t meta_arg_num{
				static_cast<bool>(flags & ut_cast(log_metaargs::File))
				+ static_cast<bool>(flags & ut_cast(log_metaargs::Line))
				+ static_cast<bool>(flags & ut_cast(log_metaargs::Function)) };
			static_assert(sizeof...(Args) > meta_arg_num, "Missing file/line/function or log message!");
			
			constexpr bool has_logger_name = flags & ut_cast(log_metaargs::Logger);
			constexpr bool has_thread_name = flags & ut_cast(log_metaargs::Thread);
			if constexpr (!has_logger_name && !has_thread_name) {
				L::template log<level, policy, flags>(args...);
			}
			if constexpr (has_logger_name && !has_thread_name) {
				L::template log<level, policy, flags, decltype(L::name())>(
					L::name(), args...);
			}
			if constexpr (!has_logger_name && has_thread_name) {
				L::template log<level, policy, flags, decltype(L::thread())>(
					L::thread(), args...);
			}
			if constexpr (has_logger_name && has_thread_name) {
				L::template log<level, policy, flags,
					decltype(L::name()), decltype(L::thread())>(
						L::name(), L::thread(), args...);
			}
		}
		template<
			template<class T> class policy,
			log_call_flag flags,
			typename... Args>
		void log(level level, Args const&... args) noexcept(
			noexcept(L{}.template log<policy, flags>(level, args...))
			&& noexcept(this->name())
			&& noexcept(this->thread()))
		{
			constexpr std::size_t meta_arg_num{
				static_cast<bool>(flags & ut_cast(log_metaargs::File))
				+ static_cast<bool>(flags & ut_cast(log_metaargs::Line))
				+ static_cast<bool>(flags & ut_cast(log_metaargs::Function)) };
			static_assert(sizeof...(Args) > meta_arg_num, "Missing metaarg or log message!");

			constexpr bool has_logger_name = flags & ut_cast(log_metaargs::Logger);
			constexpr bool has_thread_name = flags & ut_cast(log_metaargs::Thread);
			if constexpr (!has_logger_name && !has_thread_name) {
				L::template log<policy, flags>(level, args...);
			}
			if constexpr (has_logger_name && !has_thread_name) {
				L::template log<policy, flags, decltype(L::name())>(
					level, L::name(), args...);
			}
			if constexpr (!has_logger_name && has_thread_name) {
				L::template log<policy, flags, decltype(L::thread())>(
					level, L::thread(), args...);
			}
			if constexpr (has_logger_name && has_thread_name) {
				L::template log<policy, flags, decltype(L::name()), decltype(L::thread())>(
					level, L::name(), L::thread(), args...);
			}
		}
	};
}
