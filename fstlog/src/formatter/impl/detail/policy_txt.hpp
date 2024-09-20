//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <string_view>
#include <fstlog/detail/log_policy.hpp>

namespace fstlog {
    inline constexpr std::string_view policy_txt(log_policy pol) noexcept {
        switch (pol) {
        case log_policy::Guaranteed:
            return "Guaranteed";
        case log_policy::NonGuaranteed:
            return "NonGuaranteed";
        case log_policy::LowLatency:
            return "LowLatency";
        default:
            return "UNKNOWN";
        }
    }
}
