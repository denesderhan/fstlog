//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <utility>

#include <fstlog/detail/level.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
    template<class L>
    class log_try_get_buffer_mixin : public L {
    public:
        template<
            level level, 
            template<class T> class policy, 
            log_call_flag flags,  
            class... Args>
        constexpr void log(Args const&... args) noexcept(
            noexcept(std::declval<L&>().is_buffer_set())
            && noexcept(std::declval<L&>().set_buffer(std::declval<log_buffer&>()))
            && noexcept(std::declval<L&>().get_buffer(0))
            && noexcept(std::declval<L&>().template log<level, policy, flags>(args...)))
        {
            if (!L::is_buffer_set()) {
                L::set_buffer(L::get_buffer(0));
            }
            L::template log<level, policy, flags>(args...);
        }
        template<
            template<class T> class policy,
            log_call_flag flags,
            class... Args>
        constexpr void log(level level, Args const&... args) noexcept(
            noexcept(std::declval<L&>().is_buffer_set())
            && noexcept(std::declval<L&>().set_buffer(std::declval<log_buffer&>()))
            && noexcept(std::declval<L&>().get_buffer(0))
            && noexcept(std::declval<L&>().template log<policy, flags>(level, args...)))
        {
            if (!L::is_buffer_set()) {
                L::set_buffer(L::get_buffer(0));
            }
            L::template log<policy, flags>(level, args...);
        }
    };
}
