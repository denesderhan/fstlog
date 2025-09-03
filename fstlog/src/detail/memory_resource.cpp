//Copyright © 2025, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/detail/memory_resource.hpp>

namespace fstlog {
    memory_resource* get_default_resource() noexcept {
        return std::pmr::get_default_resource();
    }
}
