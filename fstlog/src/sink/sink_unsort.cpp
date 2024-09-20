//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/sink/sink_unsort.hpp>

#include <config_sink.hpp>
#include <detail/make_allocated.hpp>
#include <detail/mixin/allocator_mixin.hpp>
#include <detail/mixin/exclusive_use_mixin.hpp>
#include <detail/mixin/reference_counter_mixin.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/filter/filter.hpp>
#include <filter/filter_impl.hpp>
#include <filter/filter_internal.hpp>
#include <sink/impl/sink_unsort_mixin.hpp>
#include <filter/filter_mixin.hpp>
#include <sink/sink_flush_time_mixin.hpp>
#include <sink/sink_formatter_mixin.hpp>
#include <sink/sink_interface_mixin.hpp>
#include <sink/sink_msgblock_mixin.hpp>
#include <sink/sink_output_mixin.hpp>

namespace fstlog {
    using sink_unsort_impl_type = 
        sink_interface_mixin<
        sink_msgblock_mixin<
        sink_unsort_mixin<
        sink_formatter_mixin<2048,
        sink_output_mixin<
        filter_mixin<
        sink_flush_time_mixin<
		reference_counter_mixin<
        exclusive_use_mixin<
        allocator_mixin>>>>>>>>>;
    
	static const char* sink_unsort(
		sink& out,
		formatter formatter,
		output output,
		filter_internal const& filter,
		std::chrono::milliseconds flush_interval,
		fstlog_allocator const& allocator) noexcept
	{
		out = make_allocated<sink_unsort_impl_type>(allocator);
		sink_unsort_impl_type* const pimpl = 
			static_cast<sink_unsort_impl_type*>(out.pimpl());
		if (pimpl == nullptr) return "Allocation failed (sink obj.)!";
		auto error = pimpl->set_formatter(std::move(formatter));
		if(error == nullptr) error = pimpl->set_output(std::move(output));
		if (error != nullptr) {
			out = sink{};
			return error;
		}
		pimpl->set_filter(filter);
		pimpl->set_flush_interval(flush_interval);
		return nullptr;
	}

	const char* sink_unsort(
		sink& out,
		formatter formatter,
        output output,
        fstlog_allocator const& allocator) noexcept
	{
		out = sink{};
		constexpr filter_internal filter{ level::All, 1, 255 };
		return sink_unsort(
			out,
			std::move(formatter),
			std::move(output),
			filter,
			config::default_sink_flush_interval,
			allocator);
    }
	const char* sink_unsort(
		sink& out,
		formatter formatter, 
        output output,
        filter filter,
        fstlog_allocator const& allocator) noexcept
	{
		out = sink{};
		if (!filter.good()) return "Filter was bad!";
		return sink_unsort(
			out,
			std::move(formatter),
			std::move(output),
			filter.pimpl()->message_filter_,
			config::default_sink_flush_interval,
			allocator);
    }
	const char* sink_unsort(
		sink& out,
		formatter formatter, 
		output output,
        filter filter,
        std::chrono::milliseconds flush_interval, 
        fstlog_allocator const& allocator) noexcept
	{
		out = sink{};
		if (!filter.good()) return "Filter was bad!";
		return sink_unsort(
			out,
			std::move(formatter), 
			std::move(output),
			filter.pimpl()->message_filter_,
			flush_interval,
			allocator);
    }
}
