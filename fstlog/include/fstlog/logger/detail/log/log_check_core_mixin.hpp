//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <utility>

#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/level.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
    template<class L>
    class log_check_core_mixin : public L {
    public:
        template<
            level level, 
            template<class T> class policy, 
            log_call_flag flags,  
            class... Args>
        void log(Args const&... args)  noexcept(
            noexcept(error_if(true, error_code::none))
            && noexcept(std::declval<L&>().is_core_set())
            && noexcept(std::declval<L&>().template log<level, policy, flags>(args...)))
        {
            error_if(!L::is_core_set(), error_code::core_null);
            L::template log<level, policy, flags>(args...);
        }
        template<
            template<class T> class policy,
            log_call_flag flags,
            class... Args>
        void log(level level, Args const&... args)  noexcept(
            noexcept(error_if(true, error_code::none))
            && noexcept(std::declval<L&>().is_core_set())
            && noexcept(std::declval<L&>().template log<policy, flags>(level, args...)))
        {
            error_if(!L::is_core_set(), error_code::core_null);
            L::template log<policy, flags>(level, args...);
        }
    };
}
