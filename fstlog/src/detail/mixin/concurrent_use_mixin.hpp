//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <mutex>
#include <type_traits>

#include <fstlog/detail/memory_resource.hpp>

namespace fstlog {
    template<class L>
    class concurrent_use_mixin : public L
    {
    public:
        concurrent_use_mixin() noexcept = default;

        concurrent_use_mixin(const concurrent_use_mixin& other) noexcept(
            noexcept(other.get_memory_resource())
            && std::is_nothrow_constructible_v<
                concurrent_use_mixin, 
                const concurrent_use_mixin&, 
                memory_resource*>)
            : concurrent_use_mixin(other, other.get_memory_resource()) {}
        concurrent_use_mixin(const concurrent_use_mixin& other, memory_resource* resource) noexcept(
            std::is_nothrow_constructible_v<
                L,
                const L&,
                memory_resource*>)
            : L(static_cast<const L&>(other), resource) {}

        concurrent_use_mixin(concurrent_use_mixin&&) = delete;
        concurrent_use_mixin& operator=(const concurrent_use_mixin&) = delete;
        concurrent_use_mixin& operator=(concurrent_use_mixin&&) = delete;

        ~concurrent_use_mixin() = default;

        static constexpr bool use() noexcept {
            return true;
        }
        static void release() noexcept {}
    };
}
