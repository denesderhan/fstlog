//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <type_traits>

#include <formatter/formatter_interface.hpp>
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/memory_resource.hpp>
#include <detail/make_allocated.hpp>

namespace fstlog {
    class formatter;
    template<class L>
    class formatter_interface_mixin final :
        public L,
        public formatter_interface
    {
    
    private:
        using wrapper_type = formatter;
        
        formatter_interface_mixin() noexcept = default;

        formatter_interface_mixin(const formatter_interface_mixin& other) noexcept(
            noexcept(other.get_memory_resource())
            && std::is_nothrow_constructible_v<
                formatter_interface_mixin,
                const formatter_interface_mixin&,
                memory_resource*>)
            : formatter_interface_mixin(other, other.get_memory_resource()) {
        }

        formatter_interface_mixin(const formatter_interface_mixin& other, memory_resource* resource) noexcept(
            std::is_nothrow_constructible_v<
                L,
                const L&,
                memory_resource*>)
            : L(static_cast<const L&>(other), resource) {
        }

        formatter_interface_mixin& operator=(const formatter_interface_mixin&) = delete;
        formatter_interface_mixin(formatter_interface_mixin&&) = delete;
        formatter_interface_mixin& operator=(formatter_interface_mixin&&) = delete;
        ~formatter_interface_mixin() = default;

    public:
        byte_span format_message(
            byte_span_const in,
            byte_span out) noexcept final
        {
            return L::format_message(in, out);
        }

        // allocates and constructs a new type erased formatter object
        error_code clone(wrapper_type& out) const noexcept final;
        error_code clone(wrapper_type& out, memory_resource* resource) const noexcept final;

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
        friend auto make_allocated(
            memory_resource* resource) noexcept;
        template<class T, class... Args>
        friend auto make_allocated(
            memory_resource* resource,
            Args&&... args) noexcept;
        friend wrapper_type;
    };

    template<class L>
    error_code formatter_interface_mixin<L>::clone(wrapper_type& out) const noexcept {
        return clone(out, L::get_memory_resource());
    }

    template<class L>
    error_code formatter_interface_mixin<L>::clone(
        wrapper_type& out,
        memory_resource* resource) const noexcept {
        out = make_allocated<formatter_interface_mixin<L>>(resource, *this);
        if (out.pimpl() == nullptr) return error_code::alloc_fail;
        else return error_code::none;
    }
}
