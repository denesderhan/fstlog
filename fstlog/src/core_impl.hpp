//Copyright © 2022-present, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <limits>
#include <mutex>
#include <optional>
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
        // The memory resource must outlive any thread that logged with the core_impl object
        // TLS buffers are allocated from this resource and 
        // kept alive by the logging thread even after the core_impl object is destroyed
        [[nodiscard]] error_code init(std::string_view name, memory_resource* resource) noexcept;
   
        void start() noexcept;
        void stop() noexcept;
        [[nodiscard]] bool running() const noexcept;
        std::chrono::milliseconds poll_interval(
            std::chrono::milliseconds poll_interval) noexcept;
        [[nodiscard]] std::chrono::milliseconds poll_interval() const noexcept;
        [[nodiscard]] bool add_sink(sink new_sink) noexcept;
        [[nodiscard]] bool release_sink(sink_interface* sink_ptr) noexcept;
        void flush() noexcept;
        
        [[nodiscard]] log_buffer get_buffer(std::uint32_t buffer_size) noexcept;

        // wakes up the background thread
        // this is only for performance, buffer emptying is not guaranteed
        // if the background thread was not sleeping, it can miss a buffer check/flush
        // it does not know why it woke up (if it was asleep)
        void wakeup() noexcept {
            FSTLOG_ASSERT(id_ != 0 && "Uninitialized use of core_impl!");
            background_thread_condvar_.notify_one();
        }

        // notifies the background thread and wakes it up
        // the notification will not be missed (the producer must set 
        // the log_buffer's flush_requested_ flag to be flushed)
        void notify_data_ready() noexcept {
            FSTLOG_ASSERT(id_ != 0 && "Uninitialized use of core_impl!");
            std::lock_guard<std::mutex> grd(core_mutex_);
            if (!data_ready_) {
                data_ready_ = true;
                background_thread_condvar_.notify_one();
            }
        }

        [[nodiscard]] std::string_view name() const noexcept {
            FSTLOG_ASSERT(id_ != 0 && "Uninitialized use of core_impl!");
            return name_;
        }

        [[nodiscard]] memory_resource* get_memory_resource() const noexcept {
            FSTLOG_ASSERT(id_ != 0 && "Uninitialized use of core_impl!");
            FSTLOG_ASSERT(memory_resource_ != nullptr);
            return memory_resource_;
        }

        [[nodiscard]] std::uintmax_t id() const noexcept {
            FSTLOG_ASSERT(id_ != 0 && "Uninitialized use of core_impl!");
            return id_;
        }

        [[nodiscard]] log_buffer& tls_buffer() noexcept;

        using timer_clock = std::chrono::steady_clock;
        using timer_point = timer_clock::time_point;

    private:
        void set_memory_resource(memory_resource* resource) noexcept {
            FSTLOG_ASSERT(resource != nullptr);
            memory_resource_ = resource;
        }

        void add_reference() noexcept;
        // returns true if this was the last reference
        bool remove_reference() noexcept;
        
        // background_thread's function
        void run() noexcept;
        
        // sleep for the background thread
        // inputs: begin_time, lock for wait; output: wake-up time
        // it will schedule a wake-up for the lesser value of next_buffer_poll_
        // and next_sink_flush_
        // if both are nullopt, it will sleep indefinitely
        // returns the wake-up time
        timer_point sleep(timer_point now, std::unique_lock<std::mutex>& lock) noexcept;
        void read_buffer(log_buffer_impl& buffer, bool& sink_flush_needed) noexcept;
        void read_buffers(bool& flush_all_buffers, bool& sink_flush_needed) noexcept;
        [[nodiscard]] std::optional<timer_point> flush_sinks(bool flush_all_sinks) noexcept;
        
        struct pending_tasks {
            // true if a producer wants the background thread to consume
            // pending data from its buffer
            bool data_ready = false;
            // true if all data needs to be read from all buffers
            bool poll_all = false;    // next_buffer_poll_ expired
            // true if sink flush is needed
            bool flush_sinks = false; // next_sink_flush_ expired or stop
            // true if the background thread has to stop
            bool stop = false;
            // returns true if no task is pending
            bool empty() const noexcept {
                return !(data_ready || poll_all || flush_sinks || stop);
            }
        };

        // returns the pending tasks sent to the background thread
        // and tasks generated by expired timers
        // consumes (resets) the data_ready_, next_sink_flush_ 
        // and stop_requested_ member variables
        // must be called under locked core_mutex_ 
        pending_tasks take_pending(timer_point now) noexcept;

        // Read-only after init() cache line domain
        // 0 id_ signals uninitialzed state or failed initialization
        alignas(constants::cache_ls_nosharing) std::uintmax_t id_{ 0 };
        // this is the index of this core instance in the tls_buffers_
        int tls_buffer_index_{ -1 };
        memory_resource* memory_resource_{ nullptr };
        small_string<32> name_;

        // core_mutex_ cache line domain
        alignas(constants::cache_ls_nosharing) mutable std::mutex core_mutex_;
        std::condition_variable background_thread_condvar_;
        bool data_ready_{ false };
        bool stop_requested_{ false };
        // the next time point when all the log_buffers will be flushed
        std::optional<timer_point> next_buffer_poll_{ std::nullopt };
        // the earliest time point, when one of the sinks needs a flush
        std::optional<timer_point> next_sink_flush_{ std::nullopt };
        // background_thread_ wakeup interval in milliseconds, to poll logbuffers for data 
        std::optional<std::chrono::milliseconds> buffer_poll_interval_{ std::nullopt };

        // bufferstore_mutex_ cache line domain
        alignas(constants::cache_ls_nosharing) mutable std::mutex bufferstore_mutex_;
        dyn_array<log_buffer> bufferstore_;
        
        // sinkstore_mutex_ cache line domain
        alignas(constants::cache_ls_nosharing) std::mutex sinkstore_mutex_;
        dyn_array<sink> sinkstore_;
        
        // Background thread / lifecycle cache line domain
        alignas(constants::cache_ls_nosharing) mutable std::mutex  background_thread_mutex_;
        std::thread background_thread_;
        // the background logger never calls back into the core (deadlock)
        // it uses the low_latency policy: tries to write, if no space, 
        // increments dropped counter and returns (no wakeup() or notify_data_ready() calls)
        // it locks its mutex on a log call (concurrency)
        logger_background logger_;
        alignas(constants::cache_ls_nosharing) std::atomic<std::uintptr_t> reference_counter_{ 0 };
        
        // static member variables
        inline static std::mutex init_mutex_;
        // first id_ starts at 1
        inline static std::uintmax_t next_id_{ 1 };
        // array to track used/unused indexes of concurrent core instances
        // index != id_, indexes are reused but id_-s are unique through program lifetime
        inline static std::array<bool, config::core_instance_limit> tls_buffer_index_used_{ false };
        // array for thread safe buffers for the core instances, elements: pair<core.id_, log_buffer>
        inline static thread_local std::array<std::pair<std::uintmax_t, log_buffer>, config::core_instance_limit> 
            tls_buffers_{};
        
        static_assert(config::core_instance_limit <= 
            static_cast<std::size_t>((std::numeric_limits<decltype(tls_buffer_index_)>::max)()), "core_instance_limit too big for type!");

        friend class core;
    };
}
