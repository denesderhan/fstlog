//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <atomic>

namespace fstlog {
    template<class L>
    class exclusive_use_mixin : public L
    {
    public:
        using allocator_type = typename L::allocator_type;
        
		exclusive_use_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(exclusive_use_mixin(allocator_type{})))
			: exclusive_use_mixin(allocator_type{}) {}
		explicit exclusive_use_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
            : L(allocator) {}

		exclusive_use_mixin(const exclusive_use_mixin& other) noexcept(
			noexcept(exclusive_use_mixin::get_allocator())
			&& noexcept(exclusive_use_mixin(exclusive_use_mixin{}, allocator_type{})))
            : exclusive_use_mixin(other, other.get_allocator()) {}
		exclusive_use_mixin(const exclusive_use_mixin& other, allocator_type const& allocator) noexcept(
			noexcept(L(exclusive_use_mixin{}, allocator_type{})))
            : L(other, allocator) {}

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
