//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/output/output_cstream.hpp>

#include <detail/make_allocated.hpp>
#include <detail/mixin/allocator_mixin.hpp>
#include <detail/mixin/exclusive_use_mixin.hpp>
#include <detail/mixin/reference_counter_mixin.hpp>
#include <output/impl/out_cstream_mixin.hpp>
#include <output/output_interface_mixin.hpp>

namespace fstlog {
    using output_stream_impl_type =  
        output_interface_mixin<
        out_cstream_mixin<
		reference_counter_mixin<
        exclusive_use_mixin<
        allocator_mixin>>>>;

	const char* output_cstream(
		output& out,
        FILE* stream,
        fstlog_allocator const& allocator) noexcept
	{
		out = make_allocated<output_stream_impl_type>(allocator);
		if (out.pimpl() == nullptr) return "Allocation failed (output obj.)!";
		const auto error = static_cast<output_stream_impl_type*>(out.pimpl())->set_stream(stream);
		if (error != nullptr) out = output{};
		return error;
    }
}
