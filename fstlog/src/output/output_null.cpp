//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/output/output_null.hpp>

#include <detail/make_allocated.hpp>
#include <detail/mixin/memory_resource_mixin.hpp>
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
        memory_resource_mixin>>>>;

    error_code output_null(
        output& out, 
        memory_resource* resource) noexcept 
    {
        out = output{ make_allocated<output_null_impl_type>(resource) };
        if (out.pimpl() == nullptr) return error_code::alloc_fail;
        auto* const pimpl = static_cast<output_null_impl_type*>(out.pimpl());
        pimpl->set_memory_resource(resource);
        return error_code::none;
    }
}
