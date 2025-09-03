//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/memory_resource.hpp>

namespace fstlog {
    class memory_resource_mixin
    {
    public:
        using memory_resource_type = memory_resource;
        
        explicit memory_resource_mixin(memory_resource_type* resource) noexcept
            : resource_{ resource } {}
        memory_resource_mixin(const memory_resource_mixin& other) noexcept
            : memory_resource_mixin(other, other.get_memory_resource()) {}
        memory_resource_mixin(
            [[maybe_unused]] const memory_resource_mixin& other, 
            memory_resource_type* resource) noexcept
                : resource_{ resource } {}
        memory_resource_mixin& operator=(const memory_resource_mixin&) = delete;
        memory_resource_mixin(memory_resource_mixin&&) = delete;
        memory_resource_mixin& operator=(memory_resource_mixin&&) = delete;

        ~memory_resource_mixin() noexcept = default;

        memory_resource_type* get_memory_resource() const noexcept {
            return resource_;
        }

    private:
        memory_resource_type* resource_{ nullptr };
    };
}
