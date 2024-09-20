//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/level.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
	template<class L>
	class log_nobuffer_nolog_mixin : public L {
	public:
		template<
			level level, 
			template<class T> class policy, 
			log_call_flag flags,  
			class... Args>
		void log(Args const&... args) noexcept(
			noexcept(this->is_buffer_set())
			&& noexcept(L{}.template log<level, policy, flags>(args...)))
		{
			if (L::is_buffer_set())
				L::template log<level, policy, flags>(args...);
		}
		template<
			template<class T> class policy,
			log_call_flag flags,
			class... Args>
		void log(level level, Args const&... args) noexcept(
			noexcept(this->is_buffer_set())
			&& noexcept(L{}.template log<policy, flags>(level, args...)))
		{
			if (L::is_buffer_set())
				L::template log<policy, flags>(level, args...);
		}
	};
}
