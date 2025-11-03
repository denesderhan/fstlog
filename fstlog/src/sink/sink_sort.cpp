//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/sink/sink_sort.hpp>

#include <config_sink.hpp>
#include <detail/make_allocated.hpp>
#include <detail/mixin/memory_resource_mixin.hpp>
#include <detail/mixin/exclusive_use_mixin.hpp>
#include <detail/mixin/reference_counter_mixin.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/filter/filter.hpp>
#include <filter/filter_impl.hpp>
#include <filter/filter_internal.hpp>
#include <sink/impl/sink_sort_mixin.hpp>
#include <sink/impl/sink_unsort_mixin.hpp>
#include <sink/sink_filter_mixin.hpp>
#include <sink/sink_flush_time_mixin.hpp>
#include <sink/sink_formatter_mixin.hpp>
#include <sink/sink_interface_mixin.hpp>
#include <sink/sink_msgblock_mixin.hpp>
#include <sink/sink_output_mixin.hpp>

namespace fstlog {
    using sink_sort_impl_type = 
        sink_interface_mixin<
        sink_msgblock_mixin<
        sink_sort_mixin<
        sink_unsort_mixin<
        sink_formatter_mixin<2048,
        sink_output_mixin<
        sink_filter_mixin<
        sink_flush_time_mixin<
        reference_counter_mixin<
        exclusive_use_mixin<
        memory_resource_mixin>>>>>>>>>>;

    static error_code sink_sort(
        sink& out,
        formatter formatter,
        output output,
        filter_internal const& filter,
        std::chrono::milliseconds flush_interval,
        std::uint32_t max_buffer_bytes,
        memory_resource* resource) noexcept
    {
        out = sink{ make_allocated<sink_sort_impl_type>(resource) };
        const auto pimpl = static_cast<sink_sort_impl_type*>(out.pimpl());
        if (pimpl == nullptr) {
            return error_code::alloc_fail;
        }
        pimpl->set_memory_resource(resource); 
        pimpl->set_filter(filter);
        pimpl->set_flush_interval(flush_interval);
        auto error = pimpl->init_sink_sort(resource, max_buffer_bytes);
        if (error == error_code::none) error = pimpl->set_formatter(std::move(formatter));
        if (error == error_code::none) error = pimpl->set_output(std::move(output));
        if (error != error_code::none) {
            out = sink{};
        }
        return error;
    }

    error_code sink_sort(
        sink& out,
        formatter formatter, 
        output output,
        filter filter,
        std::chrono::milliseconds flush_interval, 
        memory_resource* resource) noexcept
    {
        return sink_sort(
            out,
            std::move(formatter), 
            std::move(output), 
            std::move(filter),
            flush_interval, 
            15 * 1024, 
            resource);
    }
    error_code sink_sort(
        sink& out,
        formatter formatter, 
        output output,
        filter filter,
        memory_resource* resource) noexcept
    {
        return sink_sort(
            out,
            std::move(formatter), 
            std::move(output),
            std::move(filter),
            config::default_sink_flush_interval, 
            15 * 1024, 
            resource);
    }
    error_code sink_sort(
        sink& out,
        formatter formatter, 
        output output,
        memory_resource* resource) noexcept
    {
        filter_internal filter{ level::All, 1, 255 };
        return sink_sort(
            out,
            std::move(formatter), 
            std::move(output),
            filter,
            config::default_sink_flush_interval, 
            15 * 1024, 
            resource);
    }
    error_code sink_sort(
        sink& out,
        formatter formatter,
        output output,
        filter filter,
        std::chrono::milliseconds flush_interval, 
        std::uint32_t max_buffer_bytes, 
        memory_resource* resource) noexcept
    {
        out = sink{};
        if (!filter.good()) return error_code::obj_null;
        return sink_sort(
            out,
            std::move(formatter),
            std::move(output),
            filter.pimpl()->message_filter_,
            flush_interval,
            max_buffer_bytes,
            resource);
    }
}
