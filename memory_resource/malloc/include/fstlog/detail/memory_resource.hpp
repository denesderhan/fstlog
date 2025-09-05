//Copyright © 2025, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>

#include <fstlog/detail/api_def.hpp>
#include <fstlog/detail/noexceptions.hpp>

namespace fstlog {
    // returns "a pointer to allocated storage with a size of at least bytes and an alignment not less than alignment"
    // on allocation error, will throw if exceptions are enabled
    // and return nullptr if exceptions are disabled
    class malloc_resource {
    public:
        [[nodiscard]] FSTLOG_API void* allocate(std::size_t bytes, std::size_t alignment)
#ifdef FSTLOG_NOEXCEPTIONS            
            noexcept
#endif
            ;

        FSTLOG_API void deallocate(
            void* ptr,
            [[maybe_unused]] std::size_t bytes,
            [[maybe_unused]] std::size_t alignment = alignof(std::max_align_t)) noexcept;
 
        FSTLOG_API bool is_equal(const malloc_resource& other) noexcept;
    };

    using memory_resource = malloc_resource;

    FSTLOG_API malloc_resource* get_default_resource() noexcept;

    FSTLOG_API bool operator ==(malloc_resource const&, malloc_resource const&) noexcept;
    FSTLOG_API bool operator !=(malloc_resource const&, malloc_resource const&) noexcept;
    
    FSTLOG_API bool memory_resource_id_match(const char* id) noexcept;

    inline bool memory_resource_identical() noexcept {
        // this id must be unique (use a hash of the .hpp .cpp files)
        return memory_resource_id_match("fstlog_malloc_22475e3881e65aae12a7ec7b63cdecc657bdc30f3b8d5cf7c734de2bb5955b06");
    }
}
