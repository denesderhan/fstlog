//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/output/output_console.hpp>

#include <iostream>

#include <detail/make_allocated.hpp>
#include <detail/mixin/allocator_mixin.hpp>
#include <detail/mixin/concurrent_use_mixin.hpp>
#include <detail/mixin/reference_counter_mixin.hpp>
#include <output/impl/out_console_mixin.hpp>
#include <output/output_interface_mixin.hpp>

namespace fstlog {
    using output_stream_impl_type =  
        output_interface_mixin<
        out_console_mixin<
        reference_counter_mixin<
        concurrent_use_mixin<
        allocator_mixin>>>>;

    error_code output_cout(
        output& out,
        fstlog_allocator const& allocator) noexcept
    {
        out = make_allocated<output_stream_impl_type>(allocator);
        if (out.pimpl() == nullptr) return error_code::alloc_fail;
        const auto error = static_cast<output_stream_impl_type*>(out.pimpl())->set_stream(&std::cout);
        if (error != error_code::none) out = output{};
        return error;
    }

    error_code output_cerr(
        output& out,
        fstlog_allocator const& allocator) noexcept
    {
        out = make_allocated<output_stream_impl_type>(allocator);
        if (out.pimpl() == nullptr) return error_code::alloc_fail;
        const auto error = static_cast<output_stream_impl_type*>(out.pimpl())->set_stream(&std::cerr);
        if (error != error_code::none) out = output{};
        return error;
    }

    error_code output_clog(
        output& out,
        fstlog_allocator const& allocator) noexcept
    {
        out = make_allocated<output_stream_impl_type>(allocator);
        if (out.pimpl() == nullptr) return error_code::alloc_fail;
        const auto error = static_cast<output_stream_impl_type*>(out.pimpl())->set_stream(&std::clog);
        if (error != error_code::none) out = output{};
        return error;
    }
}
