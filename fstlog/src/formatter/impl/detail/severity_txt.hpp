//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <string_view>
#include <fstlog/detail/level.hpp>

namespace fstlog {
    inline constexpr std::string_view severity_txt(level sev) noexcept {
        switch (sev) {
        case level::None:
            return "NONE";
        case level::Fatal:
            return "FATAL";
        case level::Error:
            return "ERROR";
        case level::Warn:
            return "WARN";
        case level::Info:
            return "INFO";
        case level::Debug:
            return "DEBUG";
        case level::Trace:
            return "TRACE";
        case level::All:
            return "ALL";
        default:
            return "UNKNOWN";
        }
    }
}
