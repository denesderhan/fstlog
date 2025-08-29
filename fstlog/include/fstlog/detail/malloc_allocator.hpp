//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/noexceptions.hpp>

#include <cstddef>
#include <cstdlib>
#include <limits>
#include <new>
#ifndef FSTLOG_NOEXCEPTIONS
#include <stdexcept>
#endif
#include <type_traits>

namespace fstlog {
    // a memory resource that on allocation error, 
    // will throw if exceptions are enabled
    // and return nullptr if exceptions are disabled
    class malloc_resource {
    public:
        static void* allocate(
            std::size_t bytes,
            std::size_t alignment = alignof(std::max_align_t)) 
#ifdef FSTLOG_NOEXCEPTIONS            
            noexcept
#endif

        {
            void* ptr{ nullptr };
#ifdef _WIN32
            ptr = _aligned_malloc(bytes, alignment);
#else
            ptr = std::aligned_alloc(alignment, bytes);
#endif
#ifndef FSTLOG_NOEXCEPTIONS
            if (ptr == nullptr) throw std::bad_alloc{};
#endif
            return ptr;
        }

        static void deallocate(
            void* ptr,
            [[maybe_unused]] std::size_t bytes,
            [[maybe_unused]] std::size_t alignment = alignof(std::max_align_t)) noexcept
        {
#ifdef _WIN32
            _aligned_free(ptr);
#else
            std::free(ptr);
#endif
        }

        static bool is_equal(const malloc_resource&) noexcept {
            return true;
        }
    };
    inline bool operator ==(malloc_resource const&, malloc_resource const&) noexcept {
        return true;
    }
    inline bool operator !=(malloc_resource const&, malloc_resource const&) noexcept {
        return false;
    }

    // an allocator that has a resource() member
    // and can be used when exceptions are disabled
    template<class T>
    class malloc_allocator {
    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using reference = T&;
        using const_reference = const T&;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        
        using propagate_on_container_copy_assignment = std::true_type;
        using propagate_on_container_move_assignment = std::true_type;
        using propagate_on_container_swap = std::true_type;
        using is_always_equal = std::true_type;

        malloc_allocator() noexcept = default;
        malloc_allocator(const malloc_allocator&) noexcept {}
        template <class U>
        malloc_allocator(const malloc_allocator<U>&) noexcept {}
        ~malloc_allocator() noexcept = default;

        template <class U>
        struct rebind {
            using other = malloc_allocator<U>;
        };
        T* address(T &value) const noexcept {
            return &value;
        }
        const T* address(const T &value) const noexcept {
            return &value;
        }

        std::size_t max_size() const noexcept {
            return (std::numeric_limits<std::size_t>::max)() / sizeof(T);
        }

        T* allocate(std::size_t num) {
            // allocate aligned memory using the resource
            void* ptr = resource()->allocate(num * sizeof(T), alignof(T));
#ifndef FSTLOG_NOEXCEPTIONS
            if (ptr == nullptr) throw std::bad_alloc{};
#else
            if (ptr == nullptr) std::abort();
#endif
            return static_cast<T*>(ptr);
        }

        void construct(T* ptr, const T& value) {
            // construct with placement new in allocated memory
            ::new(static_cast<void*>(ptr)) T(value);
        }

        void destroy(T* ptr) {
            // destroy with destructor call
            ptr->~T();
        }

        void deallocate(T* ptr, std::size_t num) noexcept {
            resource()->deallocate(ptr, num * sizeof(T), alignof(T));
        }

        malloc_resource* resource() const noexcept {
            return &resource_;
        }
        
    private:
        inline static malloc_resource resource_;
    };

    template <class T, class U>
    bool operator== (const malloc_allocator<T>&, const malloc_allocator<U>&) noexcept {
        return true;
    }
    template <class T, class U>
    bool operator!= (const malloc_allocator<T>&, const malloc_allocator<U>&) noexcept {
        return false;
    }

    using fstlog_allocator = malloc_allocator<unsigned char>;
}
