//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/level.hpp>
#include <fstlog/detail/types.hpp>
#include <filter/filter_internal.hpp>

namespace fstlog {
    template<class L>
    class filter_mixin : public L {
    public:
        using memory_resource_type = typename L::memory_resource_type;

        explicit filter_mixin(memory_resource_type* resource) noexcept(
            noexcept(L(nullptr)))
            : L(resource) {}

        filter_mixin(const filter_mixin& other) noexcept(
            noexcept(filter_mixin::get_memory_resource())
            && noexcept(filter_mixin(filter_mixin{nullptr}, nullptr)))
            : filter_mixin(other, other.get_memory_resource()) {}
        filter_mixin(const filter_mixin& other, memory_resource_type* resource) noexcept(
            noexcept(L(filter_mixin{nullptr}, nullptr))
            && noexcept(filter_internal()))
            : L(other, resource),
            message_filter_{ other.message_filter_ } {}
        filter_mixin& operator=(const filter_mixin&) = delete;
        filter_mixin(filter_mixin&&) = delete;
        filter_mixin& operator=(filter_mixin&&) = delete;

        ~filter_mixin() = default;

        void set_filter(filter_internal const& filter) noexcept {
            message_filter_ = filter;
        }

        bool filter_msg(level severity, channel_type channel) const noexcept {
            return message_filter_.filter_msg(severity, channel);
        }

        filter_internal message_filter_;
    };
}
