//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <formatter/formatter_interface.hpp>
#include <fstlog/detail/memory_resource.hpp>
#include <detail/make_allocated.hpp>

namespace fstlog {
    class formatter;
    template<class L>
    class formatter_interface_mixin final :
        public L,
        public formatter_interface
    {
    
    public:
        byte_span format_message(
            byte_span_const in,
            byte_span out) noexcept final
        {
            return L::format_message(in, out);
        }

        bool use() noexcept final {
            return L::use();
        }

        void release() noexcept final {
            L::release();
        }
        
    private:
        void add_reference() noexcept final {
            L::add_reference();
        }

        void release_referred() noexcept final {
            if (L::remove_reference()) {
                const auto resource{ L::get_memory_resource() };
                this->~formatter_interface_mixin();
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
        friend formatter;
    };
}
