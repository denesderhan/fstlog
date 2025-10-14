//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <type_traits>

#include <fstlog/logger/detail/this_thread_id.hpp>
#include <fstlog/detail/small_string.hpp>
#include <fstlog/detail/fast_to_str.hpp>

namespace fstlog::this_thread {
    inline small_string<32> get_str() noexcept {
        const auto thr_id{ fstlog::this_thread::get_id() };
        char buffer[24]{ '0' };
        return to_dec(
            static_cast<std::make_unsigned_t<decltype(thr_id)>>(thr_id),
            buffer);
    }
}
