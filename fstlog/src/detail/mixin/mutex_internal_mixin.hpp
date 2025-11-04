//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <mutex>

namespace fstlog {
    template<class L>
    class mutex_internal_mixin : public L
    {
    public:
        std::mutex& get_mutex() noexcept {
            return sync_mutex_;
        }

    private:
        std::mutex sync_mutex_;
    };
}
