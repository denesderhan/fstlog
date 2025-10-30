//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <mutex>
#include <type_traits>

#include <detail/unaligned_span.hpp>

namespace fstlog {
    template<class L>
    class output_locked_mixin
        : public L
    {
    public:
        output_locked_mixin() noexcept = default;

        output_locked_mixin(const output_locked_mixin& other) = delete;
        output_locked_mixin(output_locked_mixin&& other) = delete;
        output_locked_mixin& operator=(const output_locked_mixin& rhs) = delete;
        output_locked_mixin& operator=(output_locked_mixin&& rhs) = delete;

        ~output_locked_mixin() = default;

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
