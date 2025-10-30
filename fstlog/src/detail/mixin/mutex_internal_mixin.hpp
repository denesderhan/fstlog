//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <mutex>
#include <type_traits>

namespace fstlog {
    template<class L>
    class mutex_internal_mixin : public L
    {
    public:
        mutex_internal_mixin(memory_resource* resource) noexcept = default;

        mutex_internal_mixin(const mutex_internal_mixin& other) noexcept(
            noexcept(other.get_memory_resource())
            && std::is_nothrow_constructible_v<
                mutex_internal_mixin, 
                const mutex_internal_mixin&, 
                memory_resource*>)
            : mutex_internal_mixin(other, other.get_memory_resource()) {}
        mutex_internal_mixin(const mutex_internal_mixin& other, memory_resource* resource) noexcept(
            std::is_nothrow_constructible_v<
                L,
                const L&,
                memory_resource*>)
            : L(static_cast<const L&>(other), resource) {
        }

        mutex_internal_mixin(mutex_internal_mixin&& other) = delete;
        mutex_internal_mixin& operator=(const mutex_internal_mixin& rhs) = delete;
        mutex_internal_mixin& operator=(mutex_internal_mixin&& rhs) = delete;
        
        ~mutex_internal_mixin() = default;

        std::mutex& get_mutex() noexcept {
            return sync_mutex_;
        }

    private:
        std::mutex sync_mutex_;
    };
}
