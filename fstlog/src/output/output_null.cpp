//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/output/output_null.hpp>

#include <detail/make_allocated.hpp>
#include <detail/mixin/allocator_mixin.hpp>
#include <detail/mixin/concurrent_use_mixin.hpp>
#include <detail/mixin/reference_counter_mixin.hpp>
#include <output/impl/out_null_mixin.hpp>
#include <output/output_interface_mixin.hpp>

namespace fstlog {
    using output_null_impl_type = 
        output_interface_mixin<
        out_null_mixin<
		reference_counter_mixin<
        concurrent_use_mixin<
        allocator_mixin>>>>;

	const char* output_null(
		output& out, 
		fstlog_allocator const& allocator) noexcept 
	{
		out = make_allocated<output_null_impl_type>(allocator);
		if (out.pimpl() == nullptr) return "Allocation failed (output obj.)!";
		return nullptr;
	}
}
