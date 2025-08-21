//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#ifndef FSTLOG_POLLINTERVAL
#define FSTLOG_POLLINTERVAL 0
#endif
#ifndef FSTLOG_CORELIMIT
#define FSTLOG_CORELIMIT 16
#endif
#include <chrono>
#include <cstddef>

namespace fstlog {
    namespace config {
        // Concurrent core instance limit.
        inline constexpr std::size_t core_instance_limit{ FSTLOG_CORELIMIT };
        // Default core background thread full flush period in milliseconds.
        inline constexpr std::chrono::milliseconds default_polling_interval{ FSTLOG_POLLINTERVAL };

        static_assert(core_instance_limit >= 4 && core_instance_limit <= 128, "Invalid FSTLOG_CORELIMIT setting (4-128)!");
    }
}
