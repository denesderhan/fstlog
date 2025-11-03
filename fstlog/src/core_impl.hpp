//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <limits>
#include <mutex>
#include <string_view>
#include <thread>
#include <utility>

#include <config_core.hpp>
#include <detail/dyn_array.hpp>
#include <fstlog/core.hpp>
#include <detail/constants_src.hpp>
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/memory_resource.hpp>
#include <fstlog/detail/small_string.hpp>
#include <fstlog/sink/sink.hpp>
#include <logger/logger_background.hpp>

namespace fstlog {
    class buffer_store;
    class log_buffer_impl;
    class log_metadata;
    class alignas(constants::cache_ls_nosharing) core_impl final
    {
    public:
        core_impl() noexcept = default;
        core_impl(const core_impl&) = delete;
        core_impl(core_impl&& other) = delete;
        core_impl& operator=(const core_impl&) = delete;
        core_impl& operator=(core_impl&& other) = delete;
        ~core_impl() noexcept;
        error_code init(std::string_view name, memory_resource* resource) noexcept;
   
        void start() noexcept;
        void stop() noexcept;
        bool running() const noexcept;
        std::chrono::milliseconds poll_interval(
            std::chrono::milliseconds poll_interval) noexcept;
        std::chrono::milliseconds poll_interval() const noexcept;
        bool add_sink(sink new_sink) noexcept;
        bool release_sink(sink_interface* sink_ptr) noexcept;
        void flush() noexcept;
        
        log_buffer get_buffer(std::uint32_t buffer_size) noexcept;

        void wakeup() noexcept {
            background_thread_condvar_.notify_all();
        }

        void notify_data_ready() noexcept {
            std::lock_guard<std::mutex> grd(core_mutex_);
            if (!data_ready_) {
                data_ready_ = true;
                background_thread_condvar_.notify_all();
            }
        }

        std::string_view name() const noexcept {
            return name_;
        }

        void set_memory_resource(memory_resource* resource) noexcept {
            FSTLOG_ASSERT(resource != nullptr);
            memory_resource_ = resource;
        }

        memory_resource* get_memory_resource() const noexcept {
            FSTLOG_ASSERT(memory_resource_ != nullptr);
            return memory_resource_;
        }

        std::uintmax_t id() const noexcept {
            return id_;
        }

        log_buffer& tls_buffer() noexcept;

    private:
        void add_reference() noexcept;

        // returns true if this was the last reference
        bool remove_reference() noexcept;
                

        typedef std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> steady_msec;
        void run() noexcept;
        void read_buffer(log_buffer_impl& buffer, bool& sink_flush_needed) noexcept;
        void read_buffers(bool& flush_all_buffers, bool& sink_flush_needed) noexcept;
        steady_msec flush_sinks(bool flush_all_sinks, steady_msec current_time) noexcept;

        //ext read/write, self read/write
        alignas(constants::cache_ls_nosharing) mutable std::mutex core_mutex_;
        alignas(constants::cache_ls_nosharing) std::condition_variable background_thread_condvar_;
        bool data_ready_{ false };
        //ext read, self read, written at init
        small_string<32> name_;

        //not contended, mutex shares cache line
        alignas(constants::cache_ls_nosharing) mutable std::mutex bufferstore_mutex_;
        dyn_array<log_buffer> bufferstore_;
        //not contended, mutex shares cache line
        std::mutex sinkstore_mutex_;
        dyn_array<sink> sinkstore_;
                 
        //the next time point when all the log_buffers will be flushed
        steady_msec next_buffer_poll_{ std::chrono::time_point_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() + config::default_polling_interval) };
        //the earliest time point, when one of the sinks needs a flush
        steady_msec  next_sink_flush_{(steady_msec::max)()};
        //background_thread_ wakeup interval in milliseconds, to poll logbuffers for data 
        std::chrono::milliseconds buffer_poll_interval_{ config::default_polling_interval };
        bool background_thread_stop_requested_{ false };

        //ext read/write
        alignas(constants::cache_ls_nosharing) mutable std::mutex  background_thread_mutex_;
        std::thread background_thread_;

        logger_background logger_;

        std::atomic<std::uintptr_t> reference_counter_{ 0 };
        
        inline static std::mutex init_mutex_;
        // 0 id_ signals uninitialzed state or failed initialization
        std::uintmax_t id_{ 0 };
        // first id_ starts at 1
        inline static std::uintmax_t next_id_{ 1 };
        // array to track used/unused indexes of concurrent core instances
        // index != id_, indexes are reused but id_-s are unique through program lifetime
        static inline std::array<bool, config::core_instance_limit> tls_buffer_index_used_{ false };
        // this is the index of this core instance in the tls_buffers_
        int tls_buffer_index_{ -1 };
        
        memory_resource* memory_resource_{ nullptr };
        
        // array for thread safe buffers for the core instances, elements: pair<core.id_, log_buffer>
        static inline thread_local std::array<std::pair<std::uintmax_t, log_buffer>, config::core_instance_limit> 
            tls_buffers_{ {{0, log_buffer{}}} };
        static_assert(config::core_instance_limit <= 
            (std::numeric_limits<decltype(tls_buffer_index_)>::max)(), "core_instance_limit too big for type!");

        friend class core;
        friend class background_thread;
    };
}
