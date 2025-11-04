//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <detail/nothrow_allocate.hpp>
#include <fstlog/detail/memory_resource.hpp>
#include <output/output_interface.hpp>

namespace fstlog {
    template<class T>
    T* make_allocated(memory_resource* resource) noexcept;
    template<class T, class... Args>
    T* make_allocated(memory_resource* resource,
        Args&&... args) noexcept;
    
    class output;
    template<class L>
    class output_interface_mixin :
        public L,
        public output_interface
    {
    public:        
        void write_message(byte_span_const msg) noexcept final {
            L::write_message(msg);
        }

        void flush() noexcept final {
            L::flush();
        }

        bool use() noexcept final {
            return L::use();
        }

        void release() noexcept final {
            L::release();
        }
    protected:
        void add_reference() noexcept final {
            L::add_reference();
        }

        void release_referred() noexcept final {
            if (L::remove_reference()) {
                const auto resource{ L::get_memory_resource() };
                this->~output_interface_mixin();
                nothrow_deallocate(this, resource);
            }
        }

        template<class T>
        friend T* make_allocated(
            memory_resource* resource) noexcept;
        template<class T, class... Args>
        friend T* make_allocated(
            memory_resource* resource,
            Args&&... args) noexcept;
        friend output;
    };
}
