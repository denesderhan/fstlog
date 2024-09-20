//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <mutex>

namespace fstlog {
    template<class L>
    class mutex_internal_mixin : public L
    {
    public:
        using allocator_type = typename L::allocator_type;
        
        mutex_internal_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(mutex_internal_mixin(allocator_type{})))
			: mutex_internal_mixin(allocator_type{}) {}
		explicit mutex_internal_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
            : L(allocator) {}

        mutex_internal_mixin(const mutex_internal_mixin& other) noexcept(
			noexcept(mutex_internal_mixin::get_allocator())
			&& noexcept(mutex_internal_mixin(mutex_internal_mixin{}, allocator_type{})))
            : mutex_internal_mixin(other, other.get_allocator()) {}
        mutex_internal_mixin(const mutex_internal_mixin& other, allocator_type const& allocator) noexcept(
			noexcept(L(mutex_internal_mixin{}, allocator_type{})))
            : L(other, allocator) {}

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
