//Copyright © 2025, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/detail/memory_resource.hpp>
#include <fstlog/detail/noexceptions.hpp>

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>
#ifndef FSTLOG_NOEXCEPTIONS
#include <stdexcept>
#endif
#include <type_traits>

#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/is_pow2.hpp>

namespace fstlog {
    
    [[nodiscard]] void* malloc_resource::allocate(std::size_t bytes, std::size_t alignment)
#ifdef FSTLOG_NOEXCEPTIONS            
        noexcept
#endif
    {
        void* ptr{ nullptr };
        FSTLOG_ASSERT(is_pow2(alignment));
#ifdef _WIN32
        if (bytes == 0) bytes = 1; // Allocate 1 byte to get unique non-null pointer.
        ptr = _aligned_malloc(bytes, alignment);
#else
        auto rounded_bytes = bytes;
        if (bytes <= alignment) {
            rounded_bytes = alignment; // Allocate minimum aligned size to get unique non-null pointer
        }
        else {
            // Round up to multiple of alignment as required
            auto temp = bytes % alignment;
            if (temp != 0) {
                temp = alignment - temp; // padding
                // overflow check: if padding > available
                if (temp > (std::numeric_limits<std::size_t>::max)() - bytes) {
                    rounded_bytes = 0;
                }
                else {
                    rounded_bytes = bytes + temp; // add padding
                }
            }
        }
        if (rounded_bytes != 0) {
            ptr = std::aligned_alloc(alignment, rounded_bytes);
        }
#endif    
#ifndef FSTLOG_NOEXCEPTIONS
        if (ptr == nullptr) throw std::bad_alloc{};
#endif
        return ptr;
    }

    void malloc_resource::deallocate(
        void* ptr,
        [[maybe_unused]] std::size_t bytes,
        [[maybe_unused]] std::size_t alignment) noexcept
    {
#ifdef _WIN32
        // If ptr is a NULL pointer, this function simply performs no actions.
        _aligned_free(ptr);
#else
        // If ptr is a null pointer, the function does nothing. 
        std::free(ptr);
#endif
    }

    bool malloc_resource::is_equal(const malloc_resource& other) noexcept {
        return this == &other;
    }

    malloc_resource* get_default_resource() noexcept {
        static malloc_resource resource;
        return &resource;
    }

    bool operator ==(malloc_resource const &lhs, malloc_resource const &rhs) noexcept {
        return &lhs == &rhs;
    }
    bool operator !=(malloc_resource const &lhs, malloc_resource const &rhs) noexcept {
        return !(lhs == rhs);
    }
    
    bool memory_resource_id_match(const char* id) noexcept {
        return std::strcmp(id, "fstlog_malloc_22475e3881e65aae12a7ec7b63cdecc657bdc30f3b8d5cf7c734de2bb5955b06") == 0;
    }
}
