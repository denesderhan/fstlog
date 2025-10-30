//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/memory_resource.hpp>
#include <filter/filter_internal.hpp>

namespace fstlog {
    class filter_impl {
    public:
        using wrapper_type = filter_impl*;

        filter_impl() noexcept = default;

        filter_impl(const filter_impl& other) noexcept
            : filter_impl(other, other.get_memory_resource()) {
        }

        filter_impl(const filter_impl& other, memory_resource* resource) noexcept
            : resource_{ resource },
            message_filter_{ other.message_filter_ } {
        }
        
        bool operator==(const filter_impl& other) const noexcept {
            return message_filter_ == other.message_filter_;
        }
        bool operator!=(const filter_impl& other) const noexcept {
            return !(*this == other);
        }

        void set_filter(filter_internal const& filter) noexcept {
            message_filter_ = filter;
        }

        bool filter_msg(level severity, channel_type channel) const noexcept {
            return message_filter_.filter_msg(severity, channel);
        }

        memory_resource* get_memory_resource() const noexcept {
            return resource_;
        }

        void set_memory_resource(memory_resource* resource) noexcept {
            resource_ = resource;
        }

        filter_internal message_filter_{};
        memory_resource* resource_{ nullptr };
    };
}
