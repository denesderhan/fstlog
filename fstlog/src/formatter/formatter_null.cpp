//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/formatter/formatter_null.hpp>

#include <detail/make_allocated.hpp>
#include <detail/mixin/memory_resource_mixin.hpp>
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
        memory_resource_mixin>>>>;

    error_code formatter_null(
        formatter& out,
        memory_resource* resource) noexcept 
    {
        out = make_allocated<formatter_null_type>(resource);
        if (out.pimpl() == nullptr) return error_code::alloc_fail;
        const auto pimpl = static_cast<formatter_null_type*>(out.pimpl());
        pimpl->set_memory_resource(resource);
        return error_code::none;
    }
}
