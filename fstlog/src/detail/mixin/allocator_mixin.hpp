//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/fstlog_allocator.hpp>

namespace fstlog {
	class allocator_mixin
    {
    public:
        using allocator_type = fstlog_allocator;
        
		allocator_mixin() noexcept(
			noexcept(allocator_type()) 
			&& noexcept(allocator_type(allocator_type{})))
			: allocator_mixin(allocator_type{}) {}
		explicit allocator_mixin(allocator_type const& allocator) 
			noexcept(noexcept(allocator_type(allocator_type{})))
            : allocator_{ allocator } {}

        allocator_mixin(const allocator_mixin& other) noexcept(
			noexcept(allocator_type(allocator_type{})))
            : allocator_mixin(other, other.get_allocator()) {}
        allocator_mixin([[maybe_unused]] const allocator_mixin& other, allocator_type const& allocator) noexcept(
			noexcept(allocator_type(allocator_type{})))
            : allocator_{ allocator } {
        }
		allocator_mixin& operator=(const allocator_mixin&) = delete;
        allocator_mixin(allocator_mixin&&) = delete;
        allocator_mixin& operator=(allocator_mixin&&) = delete;

		~allocator_mixin() = default;

        allocator_type const& get_allocator() const noexcept {
            return allocator_;
        }

    private:
        const allocator_type allocator_;
    };
}
