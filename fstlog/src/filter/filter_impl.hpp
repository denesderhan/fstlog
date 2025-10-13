//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <type_traits>

#include <detail/mixin/memory_resource_mixin.hpp>
#include <filter/filter_mixin.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/memory_resource.hpp>

namespace fstlog {
    template<class T>
    auto make_allocated(memory_resource* resource) noexcept;
    template<class T, class... Args>
    auto make_allocated(memory_resource* resource,
        Args&&... args) noexcept;

    class filter_impl 
        : public filter_mixin<
                memory_resource_mixin> {
    private:
        using wrapper_type = filter_impl*;

        explicit filter_impl(memory_resource* resource) noexcept(
            std::is_nothrow_constructible_v<
                filter_mixin,
                memory_resource*>)
            : filter_mixin(resource) {}

        filter_impl(const filter_impl& other) noexcept(
            noexcept(std::declval<const filter_impl&>().get_memory_resource())
            && std::is_nothrow_constructible_v<
                filter_impl,
                const filter_impl&,
                memory_resource*>)
            : filter_impl(other, other.get_memory_resource()) {}
        filter_impl(const filter_impl& other, memory_resource* resource) noexcept(
            std::is_nothrow_constructible_v<
                filter_mixin,
                const filter_mixin&,
                memory_resource*>)
            : filter_mixin(static_cast<const filter_mixin&>(other), resource) {}
        filter_impl& operator=(const filter_impl&) = delete;
        filter_impl(filter_impl&&) = delete;
        filter_impl& operator=(filter_impl&&) = delete;
        bool operator==(const filter_impl& other) const noexcept {
            return this->message_filter_ == other.message_filter_;
        }
        bool operator!=(const filter_impl& other) const noexcept {
            return !(*this == other);
        }
        ~filter_impl() = default;
            
        template<class T>
        friend auto make_allocated(
            memory_resource* resource) noexcept;
        template<class T, class... Args>
        friend auto make_allocated(
            memory_resource* resource,
            Args&&... args) noexcept;
        friend class filter;
    };
}
