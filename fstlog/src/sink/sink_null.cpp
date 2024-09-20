//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/sink/sink_null.hpp>

#include <detail/make_allocated.hpp>
#include <detail/mixin/allocator_mixin.hpp>
#include <detail/mixin/concurrent_use_mixin.hpp>
#include <detail/mixin/reference_counter_mixin.hpp>
#include <sink/impl/sink_null_mixin.hpp>
#include <sink/sink_interface_mixin.hpp>

namespace fstlog {
    using sink_null_impl_type = 
        sink_interface_mixin<
        sink_null_mixin<
		reference_counter_mixin<
        concurrent_use_mixin<
        allocator_mixin>>>>;

    const char* sink_null(
		sink& out,
        fstlog_allocator const& allocator) noexcept
	{
        out = make_allocated<sink_null_impl_type>(allocator);
		if (out.pimpl() == nullptr) return "Allocation failed (sink obj.)!";
		return nullptr;
    }
}
