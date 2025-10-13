//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <mutex>
#include <type_traits>

namespace fstlog {
    template<class L>
    class concurrent_use_mixin : public L
    {
    public:
        using memory_resource_type = typename L::memory_resource_type;
        
        explicit concurrent_use_mixin(memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<L, memory_resource_type*>)
            : L(resource) {}

        concurrent_use_mixin(const concurrent_use_mixin& other) noexcept(
            noexcept(other.get_memory_resource())
            && std::is_nothrow_constructible_v<
                concurrent_use_mixin, 
                const concurrent_use_mixin&, 
                memory_resource_type*>)
            : concurrent_use_mixin(other, other.get_memory_resource()) {}
        concurrent_use_mixin(const concurrent_use_mixin& other, memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<
                L,
                const L&,
                memory_resource_type*>)
            : L(static_cast<const L&>(other), resource) {}

        concurrent_use_mixin(concurrent_use_mixin&& other) = delete;
        concurrent_use_mixin& operator=(const concurrent_use_mixin& rhs) = delete;
        concurrent_use_mixin& operator=(concurrent_use_mixin&& rhs) = delete;

        ~concurrent_use_mixin() = default;

        static constexpr bool use() noexcept {
            return true;
        }
        static void release() noexcept {}
    };
}
