//Copyright © 2025, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <memory_resource>

#include <fstlog/detail/api_def.hpp>

namespace fstlog {
    using memory_resource = std::pmr::memory_resource;
    
    FSTLOG_API memory_resource* get_default_resource() noexcept;
}
