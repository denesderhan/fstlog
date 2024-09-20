//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#ifndef FSTLOG_COMPILETIME_LOGLEVEL
#define FSTLOG_COMPILETIME_LOGLEVEL All
#endif
#include <fstlog/detail/level.hpp>
#include <fstlog/detail/types.hpp>
#include <fstlog/detail/ut_cast.hpp>

namespace fstlog {
	template<class L>
	class log_level_filter_mixin : public L {
	public:
		template<
			level level, 
			template<class T> class policy, 
			log_call_flag flags,  
			class... Args>
		void log(Args const&... args) noexcept(
			noexcept(this->level())
			&& noexcept(L{}.template log<level, policy, flags>(args...)))
		{
			//This if constexpr is not redundant!! (needed if not the LOG macro is used)
			if constexpr (ut_cast(level) <= ut_cast(level::FSTLOG_COMPILETIME_LOGLEVEL)) {
				if (ut_cast(level) <= ut_cast(L::level())) {
					L::template log<level, policy, flags>(args...);
#ifdef FSTLOG_FLUSH_EVERY
					L::get_core().flush();
#endif
				}
			}
		}
		template<
			template<class T> class policy,
			log_call_flag flags,
			class... Args>
		void log(level level, Args const&... args) noexcept(
			noexcept(this->level())
			&& noexcept(L{}.template log<policy, flags>(level, args...)))
		{
			if (ut_cast(level) <= ut_cast(level::FSTLOG_COMPILETIME_LOGLEVEL)
				&& ut_cast(level) <= ut_cast(L::level()))
			{
					L::template log<policy, flags>(level, args...);
#ifdef FSTLOG_FLUSH_EVERY
					L::get_core().flush();
#endif
			}
		}
	};
}
