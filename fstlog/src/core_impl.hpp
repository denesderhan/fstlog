//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>

#include <detail/dyn_array.hpp>
#include <fstlog/core.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/fstlog_allocator.hpp>
#include <fstlog/detail/small_string.hpp>
#include <fstlog/sink/sink.hpp>
#include <logger/logger_background.hpp>

namespace fstlog {
	template<class T, class... Args>
	auto make_allocated(
		fstlog_allocator const& allocator,
		Args&&... args) noexcept;
	
	class buffer_store;
    class log_buffer_impl;
    class log_metadata;
    class alignas(constants::cache_ls_nosharing) core_impl final
    {
    public:
        using allocator_type = fstlog_allocator;
	private:
		using wrapper_type = core_impl*;

        core_impl() = delete;
        core_impl(std::string_view name, allocator_type const& allocator = {}) noexcept;
        core_impl(const core_impl&) = delete;
        core_impl(core_impl&& other) = delete;
        core_impl& operator=(const core_impl&) = delete;
        core_impl& operator=(core_impl&& other) = delete;
        ~core_impl() noexcept;
		const char* init();
	public:    
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
            bool notify = false;
            {
                std::lock_guard<std::mutex> grd(core_mutex_);
                data_ready_ = true;
                if (background_thread_sleeping_) notify = true;
            }
            if (notify) background_thread_condvar_.notify_all();
        }

        std::string_view name() const noexcept {
            return name_;
        }

        allocator_type const& get_allocator() const noexcept {
            return bufferstore_.get_allocator();
        }

		static std::string_view version() noexcept;
		static int version_major() noexcept;
		static int version_minor() noexcept;
		static int version_patch() noexcept;

		std::uintmax_t id() const noexcept {
			return id_;
		}
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
        bool background_thread_sleeping_{ false };
        //ext read, self read, written at init
        small_string<32> name_;

        //not contended, mutex shares cache line
        alignas(constants::cache_ls_nosharing) mutable std::mutex bufferstore_mutex_;
        dyn_array<log_buffer, allocator_type> bufferstore_;
		//not contended, mutex shares cache line
        std::mutex sinkstore_mutex_;
        dyn_array<sink, allocator_type> sinkstore_;
                 
        //the next time point when all the log_buffers will be flushed
        steady_msec next_buffer_poll_;
        //the earliest time point, when one of the sinks needs a flush
        steady_msec  next_sink_flush_{(steady_msec::max)()};
        //background_thread_ wakeup interval in milliseconds, to poll logbuffers for data 
        std::chrono::milliseconds buffer_poll_interval_;
        bool background_thread_stop_requested_{ false };

        //ext read/write
        alignas(constants::cache_ls_nosharing) mutable std::mutex  background_thread_mutex_;
        std::thread background_thread_;

        logger_background logger_;

		std::atomic<std::uintptr_t> reference_counter_{ 0 };
		
		const std::uintmax_t id_;
		inline static std::atomic<std::uintmax_t> next_id_{ 0 };
		
		template<class T, class... Args>
		friend auto make_allocated(
			fstlog_allocator const& allocator,
			Args&&... args) noexcept;
		friend class core;
		friend class background_thread;
    };
}
