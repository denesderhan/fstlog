//Copyright © 2025, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <memory_resource>

#include <fstlog/detail/api_def.hpp>

namespace fstlog {
    using memory_resource = std::pmr::memory_resource;
    
    FSTLOG_API memory_resource* get_default_resource() noexcept;

    FSTLOG_API bool memory_resource_id_match(const char* id) noexcept;

    inline bool memory_resource_identical() noexcept {
        // this id must be unique (use a hash of the .hpp .cpp files)
        return memory_resource_id_match("fstlog_std_pmr_158c95bbc89d6b6a3bfd3e0b36b67d80dcf3d7272b5d85027828c1c53c4adb61");
    }
}
