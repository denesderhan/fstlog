//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <string_view>
#include <type_traits>

#include <fstlog/detail/memory_resource.hpp>
#include <fstlog/detail/str_hash_fnv.hpp>

namespace fstlog {
    template<typename L>
    class hash_converter_null_mixin : public L {
    public:
        hash_converter_null_mixin() noexcept = default;

        hash_converter_null_mixin(const hash_converter_null_mixin& other) noexcept(
            noexcept(other.get_memory_resource())
            && std::is_nothrow_constructible_v<
                hash_converter_null_mixin,
                const hash_converter_null_mixin&,
                memory_resource*>)
            : hash_converter_null_mixin(other, other.get_memory_resource()) {}
        hash_converter_null_mixin(const hash_converter_null_mixin& other, memory_resource* resource) noexcept(
            std::is_nothrow_constructible_v<
                L,
                const L&,
                memory_resource*>)
            : L(static_cast<const L&>(other), resource) {}

        hash_converter_null_mixin(hash_converter_null_mixin&& other) = delete;
        hash_converter_null_mixin& operator=(const hash_converter_null_mixin& rhs) = delete;
        hash_converter_null_mixin& operator=(hash_converter_null_mixin&& rhs) = delete;
        
        ~hash_converter_null_mixin() = default;

        static std::string_view convert_hash(
            [[maybe_unused]] str_hash_fnv hash) noexcept 
        {
            return std::string_view{};
        }
    };
}
