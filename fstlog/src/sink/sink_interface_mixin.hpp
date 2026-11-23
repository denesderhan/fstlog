//Copyright © 2022-present, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <chrono>
#include <cstdint>
#include <optional>

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

        using timer_point = std::chrono::steady_clock::time_point;

        error_code sink_msg_block(const unsigned char* dat_ptr, std::uint32_t dat_size) noexcept final{
            return L::sink_msg_block(dat_ptr, dat_size);
        }

        bool needs_immediate_flush() const noexcept final{
            return L::needs_immediate_flush();
        }

        std::optional<timer_point> next_flush_time() const noexcept final{
            return L::next_flush_time();
        }

        void set_next_flush_time(std::optional<timer_point> next_time) noexcept final{
            L::set_next_flush_time(next_time);
        }

        std::optional<std::chrono::milliseconds> flush_interval() const noexcept final{
            return L::flush_interval();
        }

        void set_flush_interval(std::optional<std::chrono::milliseconds> interval) noexcept final{
            L::set_flush_interval(interval);
        }

        void flush() noexcept final{
            L::flush();
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
