//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <mutex>

namespace fstlog {
    template<class L>
    class concurrent_use_mixin : public L
    {
    public:
        using allocator_type = typename L::allocator_type;
        
        concurrent_use_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(concurrent_use_mixin(allocator_type{})))
			: concurrent_use_mixin(allocator_type{}) {}
		explicit concurrent_use_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
            : L(allocator) {}

		concurrent_use_mixin(const concurrent_use_mixin& other) noexcept(
			noexcept(concurrent_use_mixin::get_allocator())
			&& noexcept(concurrent_use_mixin(concurrent_use_mixin{}, allocator_type{})))
            : concurrent_use_mixin(other, other.get_allocator()) {}
		concurrent_use_mixin(const concurrent_use_mixin& other, allocator_type const& allocator) noexcept(
			noexcept(L(concurrent_use_mixin{}, allocator_type{})))
            : L(other, allocator){
        }

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
