//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <type_traits>

#include <fstlog/detail/level.hpp>
#include <fstlog/detail/types.hpp>
#include <filter/filter_internal.hpp>

namespace fstlog {
    template<class L>
    class sink_filter_mixin : public L {
    public:
        using memory_resource_type = typename L::memory_resource_type;

        explicit sink_filter_mixin(memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<L, memory_resource_type*>)
            : L(resource) {}

        void set_filter(filter_internal const& filter) noexcept {
            message_filter_ = filter;
        }

        bool filter_msg(level severity, channel_type channel) const noexcept {
            return message_filter_.filter_msg(severity, channel);
        }

        filter_internal message_filter_;
    };
}
