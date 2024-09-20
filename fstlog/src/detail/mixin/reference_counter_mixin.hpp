//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <atomic>
#include <cstdint>

namespace fstlog {
    template<class L>
    class reference_counter_mixin : public L
    {
    public:
        using allocator_type = typename L::allocator_type;
        
		reference_counter_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(reference_counter_mixin(allocator_type{})))
			: reference_counter_mixin(allocator_type{}) {}
		explicit reference_counter_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
            : L(allocator) {}

        reference_counter_mixin(const reference_counter_mixin& other) noexcept(
			noexcept(reference_counter_mixin::get_allocator())
			&& noexcept(reference_counter_mixin(reference_counter_mixin{}, allocator_type{})))
            : reference_counter_mixin(other, other.get_allocator()) {}
		reference_counter_mixin(const reference_counter_mixin& other, allocator_type const& allocator) noexcept(
			noexcept(L(reference_counter_mixin{}, allocator_type{})))
            : L(other, allocator) {}	//ref count is not copied

        reference_counter_mixin(reference_counter_mixin&& other) = delete;
        reference_counter_mixin& operator=(const reference_counter_mixin& rhs) = delete;
        reference_counter_mixin& operator=(reference_counter_mixin&& rhs) = delete;

        ~reference_counter_mixin() = default;

		void add_reference() noexcept {
			// std::uintptr_t can not overflow if add_reference() is called from a new object only once,
			// a new object is at least std::uintptr_t size 
			// and memory would be exhausted before overflow could occur 
			reference_counter_.fetch_add(1, std::memory_order_relaxed);
		}

		// returns true if this was the last reference
        bool remove_reference() noexcept {
            const auto prev_count = 
				reference_counter_.fetch_sub(1, std::memory_order_acq_rel);
			FSTLOG_ASSERT(prev_count != 0);
			return prev_count == 1;
		}

		std::uintptr_t use_count() const noexcept {
			return reference_counter_.load(std::memory_order_relaxed);
		}

    private:
		std::atomic<std::uintptr_t> reference_counter_{ 0 };
    };
}
