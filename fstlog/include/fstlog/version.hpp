//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/api_def.hpp>

/**
 * @brief Version numbers follow Semantic Versioning (MAJOR.MINOR.PATCH)
 * @see https://semver.org
 */

/**
 * @brief Major version number of the header files
 * @warning Do not manually compare version numbers
 * @see fstlog::compatible() in @ref compatible.hpp
 * @note For major version 0, all components must match exactly
 */
#define FSTLOG_HEADER_VERSION_MAJOR 0
 
/**
 * @brief Minor version number of the header files
 * @warning Do not manually compare version numbers
 * @see fstlog::compatible() in @ref compatible.hpp
 * @note For major version 0, all components must match exactly
 */
#define FSTLOG_HEADER_VERSION_MINOR 56
  
/**
 * @brief Patch version number of the header files
 * @warning Do not manually compare version numbers
 * @see fstlog::compatible() in @ref compatible.hpp
 * @note For major version 0, all components must match exactly
 */
#define FSTLOG_HEADER_VERSION_PATCH 9

namespace fstlog {
    /**
     * @brief Get major version number of the library binary
     * @return int Major version number
     * @warning Do not manually compare version numbers
     * @see fstlog::compatible() in @ref compatible.hpp
     * @note For major version 0, all components must match exactly
     */
    FSTLOG_API int version_major() noexcept;
    
    /**
     * @brief Get minor version number of the library binary
     * @return int Minor version number
     * @warning Do not manually compare version numbers
     * @see fstlog::compatible() in @ref compatible.hpp
     * @note For major version 0, all components must match exactly
     */
    FSTLOG_API int version_minor() noexcept;
    
    /**
     * @brief Get patch version number of the library binary
     * @return int Patch version number
     * @warning Do not manually compare version numbers
     * @see fstlog::compatible() in @ref compatible.hpp
     * @note For major version 0, all components must match exactly
     */
    FSTLOG_API int version_patch() noexcept;
    
    /**
     * @brief Get full version string of the library binary
     * @return const char* Null-terminated version string
     * @warning Do not manually compare version numbers
     * @see fstlog::compatible() in @ref compatible.hpp
     * @note For major version 0, all components must match exactly
     */
    FSTLOG_API const char* version() noexcept;
}


