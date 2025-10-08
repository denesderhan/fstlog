//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <chrono>
#include <fstlog/detail/types.hpp>

namespace fstlog {
    /**
     * @class stamp_chrono_mixin
     * @brief A mixin class that provides a high-resolution timestamp function.
     *
     * This class template uses `std::chrono` to generate a timestamp. It is
     * designed to be inherited by a logger class `L` to provide timestamping
     * capabilities.
     *
     * @tparam L The base logger class to which this mixin adds functionality.
     * @tparam Clock The clock type to use for generating timestamps. Defaults to
     *               `std::chrono::system_clock`.
     */
    template<class L, class Clock = std::chrono::system_clock>
    class stamp_chrono_mixin : public L {
    public:
        /**
         * @brief Gets the current timestamp as nanoseconds since the Unix epoch.
         *
         * This function retrieves the current time from the specified `Clock`,
         * converts it to a duration in nanoseconds since the Unix epoch
         * (1970-01-01 00:00:00 UTC), and returns it as a `stamp_type`.
         *
         * @note The returned value may be negative if the system's clock is set
         *       to a time before the Unix epoch.
         * @note The precision and accuracy depend on the underlying Clock implementation.
         * 
         * @warning Due to the use of a 64-bit integer for nanoseconds, the timestamp
         *          will wrap around for dates approximately 292 years before or
         *          after the 1970 epoch. This behavior is defined for C++20 and later
         *          as two's complement wrapping.
         *
         * @return The number of nanoseconds since the Unix epoch as a `stamp_type`.
         */
        static stamp_type timestamp() noexcept {
            const auto sys_duration = Clock::now().time_since_epoch();
            const auto duration = std::chrono::duration_cast<
                std::chrono::duration<stamp_type, std::nano>>(sys_duration);
            return duration.count();
        }
    };
}
