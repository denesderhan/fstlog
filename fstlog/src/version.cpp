//Copyright © 2025, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/version.hpp>

#define FSTLOG_STRINGIFY_IMPL(X) #X
#define FSTLOG_STRINGIFY(X) FSTLOG_STRINGIFY_IMPL(X)

#define FSTLOG_VERSION FSTLOG_STRINGIFY(\
FSTLOG_HEADER_VERSION_MAJOR.\
FSTLOG_HEADER_VERSION_MINOR.\
FSTLOG_HEADER_VERSION_PATCH)

namespace fstlog {
    int version_major() noexcept {
        return FSTLOG_HEADER_VERSION_MAJOR;
    }
    
    int version_minor() noexcept {
        return FSTLOG_HEADER_VERSION_MINOR;
    }
    
    int version_patch() noexcept {
        return FSTLOG_HEADER_VERSION_PATCH;
    }
    
    const char* version() noexcept {
        return FSTLOG_VERSION;
    }
}
