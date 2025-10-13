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
    class log_policy_guaranteed : public L {
    private:
        void ensure_free_space() noexcept(
            noexcept(std::declval<L&>().message_fits())
            && noexcept(std::declval<L&>().update_buffer_state())
            && noexcept(std::declval<L&>().wait_for_buffer_flush()))
        {
            const bool slow_path{ !L::message_fits() };
            // if message is greather than half buffer size, 
            // it is possible that two loops are needed (two flush and wait)
            while (slow_path) {
                L::update_buffer_state();
                if (L::message_fits()) break;
                L::wait_for_buffer_flush();
            }
        }
    public:
        template<level level, log_call_flag flags, typename... Args>
        void log(Args const&... args) noexcept(
            noexcept(std::declval<L&>().write_pos())
            && noexcept(ensure_free_space())
            && noexcept(std::declval<L&>().template write_message<level, log_policy::Guaranteed, flags>(args...))
            && noexcept(std::declval<L&>().request_flush_if_needed(std::declval<decltype(L::write_pos())&>())))
        {
            const std::uint32_t w_pos_begin = L::write_pos();
            ensure_free_space();
            L::template write_message<level, log_policy::Guaranteed, flags>(args...);
            L::request_flush_if_needed(w_pos_begin);
        }
        template<log_call_flag flags, typename... Args>
        void log(level level, Args const&... args) noexcept(
            noexcept(std::declval<L&>().write_pos())
            && noexcept(ensure_free_space())
            && noexcept(std::declval<L&>().template write_message<log_policy::Guaranteed, flags>(level, args...))
            && noexcept(std::declval<L&>().request_flush_if_needed(std::declval<decltype(L::write_pos())&>())))
        {
            const std::uint32_t w_pos_begin = L::write_pos();
            ensure_free_space();
            L::template write_message<log_policy::Guaranteed, flags>(level, args...);
            L::request_flush_if_needed(w_pos_begin);
        }
    };
}
