//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/output/output_file.hpp>

#include <detail/make_allocated.hpp>
#include <detail/mixin/allocator_mixin.hpp>
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
        allocator_mixin>>>>;
    
    const char* output_file(
		output& out,
        const char* file_path,
        fstlog_allocator const& allocator) noexcept
	{
        return output_file(out, file_path, false, allocator);
    }
	const char* output_file(
		output& out,
        const char* file_path,
        bool truncate,
        fstlog_allocator const& allocator) noexcept
	{
        return output_file(out, file_path, truncate, 16 * 1024, allocator);
    }
	const char* output_file(
		output& out,
		const char* file_path,
		bool truncate,
		std::uint32_t buffer_size,
		fstlog_allocator const& allocator) noexcept
	{
		out = make_allocated<output_file_impl_type>(allocator);
		if (out.pimpl() == nullptr) return "Allocation failed (output obj.)!";
		const auto error = static_cast<output_file_impl_type*>(out.pimpl())->
			init_output(file_path, truncate, buffer_size);
		if (error != nullptr) out = output{};
		return error;
	}
}
