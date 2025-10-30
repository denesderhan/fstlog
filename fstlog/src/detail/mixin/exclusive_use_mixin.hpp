//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <atomic>
#include <type_traits>

#include <fstlog/detail/memory_resource.hpp>

namespace fstlog {
    template<class L>
    class exclusive_use_mixin : public L
    {
    public:
        exclusive_use_mixin() noexcept = default;

        exclusive_use_mixin(const exclusive_use_mixin& other) noexcept(
            noexcept(other.get_memory_resource())
            && std::is_nothrow_constructible_v<
                exclusive_use_mixin,
                const exclusive_use_mixin&,
                memory_resource*>)
            : exclusive_use_mixin(other, other.get_memory_resource()) {}
        exclusive_use_mixin(const exclusive_use_mixin& other, memory_resource* resource) noexcept(
            std::is_nothrow_constructible_v<
                L,
                const L&,
                memory_resource*>)
            : L(static_cast<const L&>(other), resource) {
        }

        exclusive_use_mixin(exclusive_use_mixin&& other) = delete;
        exclusive_use_mixin& operator=(const exclusive_use_mixin& rhs) = delete;
        exclusive_use_mixin& operator=(exclusive_use_mixin&& rhs) = delete;

        ~exclusive_use_mixin() = default;

        bool use() noexcept {
            bool expected{ false };
            in_use_.compare_exchange_strong(expected, true);
            return !expected;
        }

        void release() noexcept {
            in_use_.store(false);
        }

    private:
        std::atomic<bool> in_use_{ false };
    };
}
