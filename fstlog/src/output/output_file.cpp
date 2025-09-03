//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/output/output_file.hpp>

#include <detail/make_allocated.hpp>
#include <detail/mixin/memory_resource_mixin.hpp>
#include <detail/mixin/reference_counter_mixin.hpp>
#include <detail/mixin/exclusive_use_mixin.hpp>
#include <output/impl/out_file_mixin.hpp>
#include <output/output_interface_mixin.hpp>

namespace fstlog {
    using output_file_impl_type = 
        output_interface_mixin<
        out_file_mixin<
        reference_counter_mixin<
        exclusive_use_mixin<
        memory_resource_mixin>>>>;
    
    error_code output_file(
        output& out,
        const char* file_path,
        memory_resource* resource) noexcept
    {
        return output_file(out, file_path, false, resource);
    }
    error_code output_file(
        output& out,
        const char* file_path,
        bool truncate,
        memory_resource* resource) noexcept
    {
        return output_file(out, file_path, truncate, 16 * 1024, resource);
    }
    error_code output_file(
        output& out,
        const char* file_path,
        bool truncate,
        std::uint32_t buffer_size,
        memory_resource* resource) noexcept
    {
        out = make_allocated<output_file_impl_type>(resource);
        if (out.pimpl() == nullptr) return error_code::alloc_fail;
        const auto error = static_cast<output_file_impl_type*>(out.pimpl())->
            init_output(file_path, truncate, buffer_size);
        if (error != error_code::none) out = output{};
        return error;
    }
}
