//Copyright © 2022-present, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <chrono>
#include <cstdint>
#include <optional>

#include <fstlog/detail/error_code.hpp>

namespace fstlog {
    class sink_interface
    {
        using timer_point = std::chrono::steady_clock::time_point;
    
    public:
        sink_interface() noexcept = default;
        sink_interface(const sink_interface&) = delete;
        sink_interface& operator=(const sink_interface&) = delete;
        sink_interface(sink_interface&&) = delete;
        sink_interface& operator=(sink_interface&&) = delete;
        virtual ~sink_interface() = default;
      
        virtual error_code sink_msg_block(const unsigned char* dat_ptr, std::uint32_t dat_size) noexcept = 0;
        virtual bool needs_immediate_flush() const noexcept = 0;
        
        virtual std::optional<std::chrono::milliseconds> flush_interval() const noexcept = 0;
        virtual void set_flush_interval(std::optional<std::chrono::milliseconds> next_time) noexcept = 0;
        virtual std::optional<timer_point> next_flush_time() const noexcept = 0;
        virtual void set_next_flush_time(std::optional<timer_point> next_time) noexcept = 0;
        virtual void flush() noexcept = 0;
        virtual bool use() noexcept = 0;
        virtual void release() noexcept = 0;
    
    private:
        virtual void add_reference() noexcept = 0;
        virtual void release_referred() noexcept = 0;
        friend class sink;
    };
} // namespace fstlog
