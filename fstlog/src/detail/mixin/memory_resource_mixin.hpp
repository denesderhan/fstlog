//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/memory_resource.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
    class memory_resource_mixin
    {
    public:
        memory_resource_mixin() noexcept = default;
        memory_resource_mixin(const memory_resource_mixin&) noexcept = default;
        memory_resource_mixin(
            [[maybe_unused]] const memory_resource_mixin& other, 
            memory_resource* resource) noexcept
                : resource_{ resource } {}
        memory_resource_mixin(memory_resource_mixin&&) noexcept = default;
        memory_resource_mixin& operator=(const memory_resource_mixin&) noexcept = default;
        memory_resource_mixin& operator=(memory_resource_mixin&&) noexcept = default;

        ~memory_resource_mixin() noexcept = default;


        void set_memory_resource(memory_resource* resource) noexcept {
            FSTLOG_ASSERT(resource_ == nullptr);
            resource_ = resource;
        }

        memory_resource* get_memory_resource() const noexcept {
            FSTLOG_ASSERT(resource_ != nullptr);
            return resource_;
        }

    private:
        memory_resource* resource_{ nullptr };
    };
}
