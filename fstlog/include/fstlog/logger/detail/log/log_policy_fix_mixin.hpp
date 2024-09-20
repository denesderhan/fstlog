//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <type_traits>

#include <fstlog/detail/level.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
	template<template<class T> class P, class L>
	class log_policy_fix_mixin : public L {
	public:
		template<
			level level,
			template<class T> class policy,
			log_call_flag flags, 
			typename... Args>
		void log(Args const&... args) noexcept(
			noexcept(static_cast<P<log_policy_fix_mixin>*>(this)->
				template log<level, flags>(args...)))
		{
			static_assert(std::is_same_v<P<L>, policy<L>>, 
				"Trying to use invalid policy!");
			static_cast<P<log_policy_fix_mixin>*>(this)->
				template log<level, flags>(args...);
		}
		template<
			template<class T> class policy,
			log_call_flag flags,
			typename... Args>
		void log(level level, Args const&... args) noexcept(
			noexcept(static_cast<P<log_policy_fix_mixin>*>(this)->
				template log<flags>(level, args...)))
		{
			static_assert(std::is_same_v<P<L>, policy<L>>, 
				"Trying to use invalid policy!");
			static_cast<P<log_policy_fix_mixin>*>(this)->
				template log<flags>(level, args...);
		}
	};
}
