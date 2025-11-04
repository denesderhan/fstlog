//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <string_view>

#include <fstlog/detail/str_hash_fnv.hpp>

namespace fstlog {
    template<typename L>
    class hash_converter_null_mixin : public L {
    public:
        static std::string_view convert_hash(
            [[maybe_unused]] str_hash_fnv hash) noexcept 
        {
            return std::string_view{};
        }
    };
}
