//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstdint>
#include <limits>

#include <detail/safe_reinterpret_cast.hpp>
#include <fstlog/detail/memory_resource.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/is_pow2.hpp>
#include <fstlog/detail/noexceptions.hpp>

namespace fstlog {
    inline void aligned_nothrow_deallocate(
        void* obj_ptr,
        memory_resource* resource,
        std::size_t byte_num,
        std::size_t alignment) noexcept
    {
        FSTLOG_ASSERT(
            resource != nullptr
            && is_pow2(alignment)
            && safe_reinterpret_cast<std::uintptr_t>(obj_ptr) % alignment == 0);
#ifdef FSTLOG_NOEXCEPTIONS
        // if allocation fails and the resource uses exceptions
        // process will abort (std::pmr::memory_resource)
        // the fstlog::malloc_resource will return nullptr
        resource->deallocate(
            obj_ptr,
            byte_num,
            alignment);
#else
        try {
            resource->deallocate(
                obj_ptr,
                byte_num,
                alignment);
        }
        catch (...) {}
        // deallocate should never throw but we defensively catch and
        // supress throws (we don't want to crash the process with the logger)
#endif
    }

    inline void* aligned_nothrow_allocate(
        memory_resource* resource, 
        std::size_t byte_num, 
        std::size_t alignment) noexcept
    {
        void* out_ptr{ nullptr };
        if (resource == nullptr 
            || !is_pow2(alignment))
        {
            return out_ptr;
        }
#ifdef FSTLOG_NOEXCEPTIONS
        // if allocation fails and the resource uses exceptions
        // process will abort (std::pmr::memory_resource)
        // the fstlog::malloc_resource will return nullptr
        out_ptr = resource->allocate(
            byte_num,
            alignment);
#else
        try {
            // allocate with provided memory_resource
            out_ptr = resource->allocate(
                byte_num,
                alignment);
        }
        catch (...) {
            // if allocate throws out_ptr must be nullptr
            FSTLOG_ASSERT(out_ptr == nullptr);
        }
#endif
        FSTLOG_ASSERT(out_ptr == nullptr ||
            safe_reinterpret_cast<std::uintptr_t>(out_ptr) % alignment == 0);
        if (out_ptr != nullptr && 
            safe_reinterpret_cast<std::uintptr_t>(out_ptr) % alignment != 0)
        {
            aligned_nothrow_deallocate(out_ptr, resource, byte_num, alignment);
            out_ptr = nullptr;
        }
        return out_ptr;
    }

    template<class T>
    T* nothrow_allocate(
        memory_resource* resource,
        std::size_t num = 1) noexcept
    {
        if (num > (std::numeric_limits<std::size_t>::max)() / sizeof(T)) {
            return nullptr;
        }
        return static_cast<T*>(aligned_nothrow_allocate(
            resource, 
            sizeof(T) * num, 
            alignof(T)));
    }
    
    template<class T>
    void nothrow_deallocate(
        T* obj_ptr,
        memory_resource* resource,
        std::size_t num = 1) noexcept
    {
        FSTLOG_ASSERT(num <= (std::numeric_limits<std::size_t>::max)() / sizeof(T));
        aligned_nothrow_deallocate(obj_ptr, resource, sizeof(T) * num, alignof(T));
    }
}
