//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/version.hpp>
#include <fstlog/core.hpp>

namespace fstlog {
    /*
    * @brief Checks API compatibility between headers and library binary using Semantic Versioning.
    *
    * @details
    * This function verifies that the compiled library binary is API-compatible with the headers
    * used at compile time. It follows Semantic Versioning (SemVer) rules:
    * - For pre-release versions (major == 0), exact version match is required (major.minor.patch).
    * - For stable releases (major >= 1), only major version must match, and library minor version
    *   must be equal or greater than header minor version (backward compatibility).
    *
    * Note: ABI compatibility is NOT guaranteed by this function. To ensure ABI compatibility,
    * use the same toolchain (compiler, stdlib, config) for application, fstlog library, and all dependencies.
    *
    * @return bool true if API compatible, false otherwise.
    */
    inline bool compatible() noexcept {
        // alpha, beta versions have to match exactly
        if constexpr (FSTLOG_VERSION_MAJOR == 0) {
            return core::version_major() == FSTLOG_VERSION_MAJOR
                && core::version_minor() == FSTLOG_VERSION_MINOR
                && core::version_patch() == FSTLOG_VERSION_PATCH;
        }
        // backwards compatibility
        else {
            return core::version_major() == FSTLOG_VERSION_MAJOR
                && core::version_minor() >= FSTLOG_VERSION_MINOR;
        }
    }
}
