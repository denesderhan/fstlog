//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/level.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
	template<class L>
	class log_policy_mixin : public L {
	public:
		template<
			level level, 
			template<class T> class policy, 
			log_call_flag flags, 
			typename... Args>
		void log(Args const&... args)  noexcept(
			noexcept(static_cast<policy<log_policy_mixin>*>(this)-> template log<level, flags>(args...)))
		{
			static_cast<policy<log_policy_mixin>*>(this)->
				template log<level, flags>(args...);
		}
		template<
			template<class T> class policy,
			log_call_flag flags,
			typename... Args>
		void log(level level, Args const&... args) noexcept(
			noexcept(static_cast<policy<log_policy_mixin>*>(this)-> template log<flags>(level, args...)))
		{
			static_cast<policy<log_policy_mixin>*>(this)->
				template log<flags>(level, args...);
		}
	};
}
