//Copyright © 2025, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <new>

namespace fstlog {
    namespace constants {
#if defined __cpp_lib_hardware_interference_size && __cpp_lib_hardware_interference_size >= 201703L
        inline constexpr std::size_t cache_ls_nosharing{ std::hardware_destructive_interference_size };
#else
        inline constexpr std::size_t cache_ls_nosharing{ 64 };
#endif
    }
}
