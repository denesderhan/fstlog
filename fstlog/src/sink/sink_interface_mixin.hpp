//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <sink/sink_interface.hpp>
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/memory_resource.hpp>
#include <detail/nothrow_allocate.hpp>

namespace fstlog {
    class sink;
    template<class L>
    class sink_interface_mixin final :
        public L,
        public sink_interface
    {
    public:        
        typedef std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> steady_msec;

        error_code sink_msg_block(const unsigned char* dat_ptr, std::uint32_t dat_size) noexcept final{
            return L::sink_msg_block(dat_ptr, dat_size);
        }

        bool needs_immediate_flush() const noexcept final{
            return L::needs_immediate_flush();
        }

        steady_msec next_flush_time() const noexcept final{
            return L::next_flush_time();
        }

        void flush(steady_msec current_time) noexcept final{
            L::flush(current_time);
        }

        bool use() noexcept final{
            return L::use();
        }

        void release() noexcept final{
            L::release();
        }

    private:
        void add_reference() noexcept final {
            L::add_reference();
        }

        void release_referred() noexcept final {
            if (L::remove_reference()) {
                const auto resource{ L::get_memory_resource() };
                this->~sink_interface_mixin();
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
        friend sink;
    };
}
