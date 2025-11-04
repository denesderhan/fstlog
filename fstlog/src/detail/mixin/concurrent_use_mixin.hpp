//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once

namespace fstlog {
    template<class L>
    class concurrent_use_mixin : public L
    {
    public:
        static bool use() noexcept {
            return true;
        }
        
        static void release() noexcept {}
    };
}
