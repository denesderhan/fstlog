//Copyright © 2025, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/detail/memory_resource.hpp>

#include <cstring>

namespace fstlog {
    memory_resource* get_default_resource() noexcept {
        return std::pmr::get_default_resource();
    }

    bool memory_resource_id_match(const char* id) noexcept {
        return std::strcmp(id, "fstlog_std_pmr_158c95bbc89d6b6a3bfd3e0b36b67d80dcf3d7272b5d85027828c1c53c4adb61") == 0;
    }
}
