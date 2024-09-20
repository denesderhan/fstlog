//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/formatter/formatter_null.hpp>

#include <detail/make_allocated.hpp>
#include <detail/mixin/allocator_mixin.hpp>
#include <detail/mixin/concurrent_use_mixin.hpp>
#include <detail/mixin/reference_counter_mixin.hpp>
#include <formatter/formatter_interface_mixin.hpp>
#include <formatter/impl/formatter_null_mixin.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
    using formatter_null_type = 
        formatter_interface_mixin<
        formatter_null_mixin<
		concurrent_use_mixin<
		reference_counter_mixin<
        allocator_mixin>>>>;

    const char* formatter_null(
		formatter& out,
		fstlog_allocator const& allocator) noexcept 
	{
		out = make_allocated<formatter_null_type>(allocator);
		if (out.pimpl() == nullptr) return "Allocation failed (formatter obj.)!";
		return nullptr;
    }
}
