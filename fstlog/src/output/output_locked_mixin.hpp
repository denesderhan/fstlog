//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <mutex>

#include <detail/unaligned_span.hpp>

namespace fstlog {
    template<class L>
    class output_locked_mixin
        : public L
    {
    public:
        void write_message(byte_span_const msg) noexcept {
            std::lock_guard<decltype(L::get_mutex())> guard_instance{ L::get_mutex() };
            L::write_message(msg);
        }

        void flush() noexcept {
            std::lock_guard<decltype(L::get_mutex())> guard_instance{ L::get_mutex() };
            L::flush();
        }
        
        void try_reopen() noexcept {
            std::lock_guard<decltype(L::get_mutex())> guard_instance{ L::get_mutex() };
            L::try_reopen();
        }
    };
}
