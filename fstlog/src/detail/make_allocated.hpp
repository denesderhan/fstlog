//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <new>
#include <utility>

#include <fstlog/detail/noexceptions.hpp>
#include <detail/nothrow_allocate.hpp>

namespace fstlog {
    template<class T>
    auto make_allocated(
        memory_resource* resource) noexcept
    {
        T* obj_ptr = nothrow_allocate<T>(resource);
        if (obj_ptr != nullptr) {
#ifdef FSTLOG_NOEXCEPTIONS
            static_assert(noexcept(T(resource)), "Constructor must be noexcept!");
            // storing the memory_resource in the object!
            obj_ptr = ::new(static_cast<void*>(obj_ptr)) T(resource);
#else
            try {
                // storing the memory_resource in the object!
                obj_ptr = ::new(static_cast<void*>(obj_ptr)) T(resource);
            }
            catch (...) {
                //constructor failed cleaning up
                nothrow_deallocate(obj_ptr, resource);
                obj_ptr = nullptr;
            }
#endif
        }
        return typename T::wrapper_type{ obj_ptr };
    }

    template<class T, class... Args>
    auto make_allocated(
        memory_resource* resource, 
        Args&&... args) noexcept
    {
        T* obj_ptr = nothrow_allocate<T>(resource);
        if (obj_ptr != nullptr) {
#ifdef FSTLOG_NOEXCEPTIONS
            static_assert(noexcept(T(std::forward<Args>(args)..., resource)),
                "Constructor must be noexcept!");
            // storing the memory_resource in the object!
            obj_ptr = ::new(static_cast<void*>(obj_ptr))
                T(std::forward<Args>(args)..., resource);
#else
            try {
                // storing the memory_resource in the object!
                obj_ptr = ::new(static_cast<void*>(obj_ptr))
                    T(std::forward<Args>(args)..., resource);
            }
            catch (...) {
                //constructor failed cleaning up
                nothrow_deallocate(obj_ptr, resource);
                obj_ptr = nullptr;
            }
#endif
        }
        return typename T::wrapper_type{ obj_ptr };
    }
}
