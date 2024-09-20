//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <core_impl.hpp>

#include <config_core.hpp>
#include <detail/log_buffer_impl.hpp>
#include <detail/log_buffer_unread_data.hpp>
#include <detail/make_allocated.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/log_buffer.hpp>
#include <fstlog/detail/noexceptions.hpp>
#include <fstlog/sink/sink.hpp>
#include <fstlog/version.hpp>
#include <logger/log_macro_background.hpp>
#include <sink/sink_interface.hpp>

namespace fstlog {
	core_impl::core_impl(std::string_view name, allocator_type const& allocator) noexcept
		: name_(name),
		id_{next_id_.fetch_add(1, std::memory_order_relaxed)},
        bufferstore_(64, allocator),
        sinkstore_(8, allocator),
		next_buffer_poll_{ std::chrono::time_point_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() + config::default_polling_interval) },
		buffer_poll_interval_{ config::default_polling_interval }
    {
        if (buffer_poll_interval_ == std::chrono::milliseconds{0})
            next_buffer_poll_ = (steady_msec::max)();
    }

    core_impl::~core_impl() noexcept {
        stop();
		std::lock_guard<std::mutex> s_guard(sinkstore_mutex_);
		for (auto& sink : sinkstore_)
			sink.pimpl()->release();
    }

	const char* core_impl::init() {
		if constexpr (level::FSTLOG_COMPILETIME_LOGLEVEL != level::None) {
#ifdef FSTLOG_DEBUG
			logger_.init(get_buffer(128 * 1024));
#else
			logger_.init(get_buffer(4 * 1024));
#endif // FSTLOG_DEBUG
		}
		start();
		if (!running()) return "Starting background thread failed!";
		if (bufferstore_.capacity() < 64 || sinkstore_.capacity() < 8)
			return "Core, internal memory allocation failure!";
		return nullptr;
	}

	bool core_impl::running() const noexcept {
		std::lock_guard<std::mutex> s_guard(background_thread_mutex_);
		return background_thread_.joinable();
	}

    void core_impl::start() noexcept
    {
		std::lock_guard<std::mutex> s_guard(background_thread_mutex_);

		if (std::this_thread::get_id() == background_thread_.get_id()) {
			LOG_LL_ERROR(logger_, "Core: {}, background thread tried to start itself!", name());
			return;
		}

		if (background_thread_.joinable()) {
			LOG_LL_WARN(logger_, "Core: {}, background thread already running.", name());
			return;
		}
						
        {
            //locking is unnecessary (thread is not running) but does not hurt
            std::lock_guard<std::mutex> c_guard(core_mutex_);
            background_thread_stop_requested_ = false;
        }
#ifdef FSTLOG_NOEXCEPTIONS
		std::thread temp(&core_impl::run, this);
		background_thread_.swap(temp);
#else
		try{
			std::thread temp(&core_impl::run, this);
			background_thread_.swap(temp);
		}
		catch(...) {
			//background_thread_ will reamain stopped
		}
#endif
    }

	void core_impl::stop() noexcept
	{
		std::lock_guard<std::mutex> s_guard(background_thread_mutex_);

		if (!background_thread_.joinable()) {
			LOG_LL_WARN(logger_, "Core: {}, background thread already stopped.", name());
			return;
		}

		if (std::this_thread::get_id() == background_thread_.get_id()) {
			LOG_LL_ERROR(logger_, "Core: {}, background thread tried to stop itself!", name());
			return;
		}

		{
			std::lock_guard<std::mutex> c_guard(core_mutex_);
			background_thread_stop_requested_ = true;
		}
		background_thread_condvar_.notify_all();
#ifdef FSTLOG_NOEXCEPTIONS
		background_thread_.join();
#else
		try {
			background_thread_.join();
		}
		catch (...) {
			//supressing
		}
#endif
	}

    // Returns poll interval for log_buffers.
	std::chrono::milliseconds core_impl::poll_interval() const noexcept {
        std::lock_guard<std::mutex> c_guard(core_mutex_);
        return buffer_poll_interval_;
    }

	// Sets poll interval for log_buffers, returns previous value.
    std::chrono::milliseconds core_impl::poll_interval(std::chrono::milliseconds poll_interval) noexcept {
        std::chrono::milliseconds prev_interval;
        {
            std::lock_guard<std::mutex> c_guard(core_mutex_);
            prev_interval = buffer_poll_interval_;
            buffer_poll_interval_ = poll_interval;
            next_buffer_poll_ = poll_interval != std::chrono::milliseconds{0} ?
                std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now())
                : (steady_msec::max)();
        }
        background_thread_condvar_.notify_all();
        LOG_LL_INFO(logger_, "Core: {}, Polling interval set to {} milliseconds.", name(), poll_interval.count());
        return prev_interval;
    }

    bool core_impl::add_sink(sink new_sink) noexcept {
		const auto sink_ptr = new_sink.pimpl();
		if (sink_ptr == nullptr) return false;
		steady_msec sink_flush_time{};
		{
			std::lock_guard<std::mutex> ss_guard(sinkstore_mutex_);
			std::size_t ind{ 0 };
			std::size_t sink_num{ sinkstore_.size() };
			while (ind < sink_num 
				&& sinkstore_[ind].pimpl() != sink_ptr) 
			{
				ind++;
			}
			if (ind != sink_num) {
				LOG_LL_WARN(logger_,
					"Core: {} failed to add sink: {:p}, sink is already used by this core!",
					name(),
					sink_ptr);
				return false;
			}
			if (!sink_ptr->use()) {
				LOG_LL_WARN(logger_,
					"Core: {} failed to add sink: {:p}, sink is used by another core!",
					name(),
					sink_ptr);
				return false;
			}
			// Sink is not used by any core (use() succeeded).
			// next_flush_time() call protected by sink_mutex and sinkstore_mutex
			sink_flush_time = sink_ptr->next_flush_time();
			if (!sinkstore_.try_push_back(new_sink)) {
				sink_ptr->release();
				return false;
			}
		}
        
        bool sink_flush_change = false;
        {
            std::lock_guard<std::mutex> c_guard(core_mutex_);
            if (sink_flush_time < next_sink_flush_) {
                sink_flush_change = true;
                // If another thread calls core.flush() concurrently
				// it is possible that here we set the time to an earlier value
				// (it is OK core will just wake up early).
				// If another sink is added concurrently with an earlier flush time
				// here we wont set next_sink_flush to a later time (sink_flush_time < next_sink_flush_)
				next_sink_flush_ = sink_flush_time;
            }
        }
        if (sink_flush_change) {
            background_thread_condvar_.notify_all();
            LOG_LL_TRACE(logger_, "Setting next_sink_flush_ to take place in : {} millisecs.",
                (sink_flush_time -
                    std::chrono::time_point_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now())
                    ).count());
        }
        LOG_LL_TRACE(logger_, "Core: {} added sink: {:p}.", name(), sink_ptr);
		return true;
    }

    // Removes sink from core, sink will miss all unflushed logs.
	// Calling core.flush() or core.stop() prior to, minimizes log loss.
	// To prevent log loss, add a new sink (with same filter), call flush() then release old sink.
	bool core_impl::release_sink(sink_interface* sink_ptr) noexcept {
        {
            LOG_LL_TRACE(logger_, "Core: {} Trying to release sink: {:p}.", name(), sink_ptr);
            sink sink_to_release;
            {
                std::lock_guard<std::mutex> ss_guard(sinkstore_mutex_);
				const auto sink_num{ sinkstore_.size() };
				for (std::size_t ind = 0; ind < sink_num; ind++) {
					if (sink_ptr == sinkstore_[ind].pimpl()) {
						//moving destruction out of locked mutex section
						sink_to_release = std::move(sinkstore_[ind]);
						sinkstore_.remove(ind);
						break;
					}
				}
            }
            if (sink_to_release.pimpl() != nullptr) {
                auto current_time = std::chrono::time_point_cast<
                    std::chrono::milliseconds>
                    (std::chrono::steady_clock::now());
				sink_to_release.pimpl()->flush(current_time);
                sink_to_release.pimpl()->release();
                LOG_LL_TRACE(logger_, "Core: {}, released sink {:p}.", name(), sink_ptr);
				return true;
            }
            else {
                LOG_LL_WARN(logger_, "Core: {}, failed to release sink{:p}, not used by this core!", name(), sink_ptr);
				return false;
			}
        }
    }

	void core_impl::flush() noexcept {
		bool flush_all_buffers{ true };
		bool sink_flush_needed{ true };
		read_buffers(flush_all_buffers, sink_flush_needed);
		const auto current_time =
			std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now());
		auto new_flush_time = flush_sinks(true, current_time);
		steady_msec new_buffer_poll;
		{
			std::lock_guard<std::mutex> c_guard(core_mutex_);
			next_sink_flush_ = new_flush_time;
			if (buffer_poll_interval_ != std::chrono::milliseconds{ 0 })
				next_buffer_poll_ = current_time + buffer_poll_interval_;
			new_buffer_poll = next_buffer_poll_;
		}
		LOG_LL_TRACE(logger_, "Flushing core, next buffer poll: {}, next sink_flush: {} in milliseconds.",
			std::chrono::duration_cast<std::chrono::milliseconds>(new_buffer_poll - current_time).count(),
			std::chrono::duration_cast<std::chrono::milliseconds>(new_flush_time - current_time).count());
		// Calling flush() can only schedule core wakup to a later time, 
		// (buffer_poll_interval and sink_flush_intervals are not changed) 
		// core can wake up before newly scheduled time and go back to sleep
	}

    log_buffer core_impl::get_buffer(std::uint32_t buffer_size) noexcept {
		auto out{ make_allocated<log_buffer_impl>(get_allocator(), buffer_size) };
		if (out.good()) {
			std::lock_guard<std::mutex> bs_guard(bufferstore_mutex_);
			if (bufferstore_.try_push_back(out)) {
				return out;
			}
		}
		//out will be deallocated
		return log_buffer{};
    }

	void core_impl::read_buffer(
		log_buffer_impl& buffer, 
		bool& sink_flush_needed) noexcept 
	{
		log_buffer_unread_data to_read;
		buffer.get_unread(to_read);
		if (to_read.size1 == 0) return;
		FSTLOG_ASSERT(to_read.pos1 != nullptr);
		std::lock_guard<std::mutex> ss_guard(sinkstore_mutex_);
		for (auto& s : sinkstore_) {
			const auto err1 = s.pimpl()->sink_msg_block(to_read.pos1, to_read.size1);
			if constexpr (ut_cast(level::FSTLOG_COMPILETIME_LOGLEVEL) != ut_cast(level::None)) {
				if (err1 == error_code::none) {
					LOG_LL_TRACE(logger_,
						"Core: {}, reading from buffer {:p} ({:4} Kb): {:6} bytes. ptr: {} (to sink: {})",
						name(), &buffer, buffer.size() / 1024,
						to_read.size1, to_read.pos1, s.pimpl());
				}
				else {
					LOG_LL_ERROR(logger_,
						"Core: {}, failed reading from buffer {:p}, error: {}, ({:4} Kb) {:6} bytes. ptr: {} (to sink: {})",
						name(), &buffer, ut_cast(err1),	buffer.size() / 1024, 
						to_read.size1, to_read.pos1, s.pimpl());
				}
			}
			if (to_read.size2 != 0) {
				FSTLOG_ASSERT(to_read.pos2 != nullptr);
				const auto err2 = s.pimpl()->sink_msg_block(to_read.pos2, to_read.size2);
				if constexpr (ut_cast(level::FSTLOG_COMPILETIME_LOGLEVEL) != ut_cast(level::None)) {
					if (err2 == error_code::none) {
						LOG_LL_TRACE(logger_,
							"Core: {}, reading from buffer {:p} ({:4} Kb): {:6} bytes. ptr: {} (to sink: {})",
							name(), &buffer, buffer.size() / 1024,
							to_read.size2, to_read.pos2, s.pimpl());
					}
					else {
						LOG_LL_ERROR(logger_,
							"Core: {}, failed reading from buffer {:p}, error: {}, ({:4} Kb) {:6} bytes. ptr: {} (to sink: {})",
							name(), &buffer, ut_cast(err2), buffer.size() / 1024,
							to_read.size2, to_read.pos2, s.pimpl());
					}
				}
			}
			if (!sink_flush_needed && s.pimpl()->needs_immediate_flush()) sink_flush_needed = true;
		}
		buffer.advance_read_pos(to_read.size1 + to_read.size2);
	}

    void core_impl::read_buffers(
		bool& flush_all_buffers, 
		bool& sink_flush_needed) noexcept 
	{
		bool sink_flush{ sink_flush_needed };
		bool flush_all = sink_flush ? true : flush_all_buffers;
		std::lock_guard<std::mutex> bs_guard(bufferstore_mutex_);
		auto buffer_num{ bufferstore_.size() };
		std::size_t buff_ind{ 0 };
		while (buff_ind < buffer_num) {
			log_buffer_impl& buffer = *bufferstore_[buff_ind].pimpl();
			const bool buffer_unused = buffer.use_count() == 1;
			const bool flush_requested = buffer.flush_requested();
			const bool buff_half_full = buffer.half_full();
			const bool flushing{ flush_all || buff_half_full || flush_requested || buffer_unused };
			if (flushing) {
				read_buffer(buffer, sink_flush);
				if (flush_requested) buffer.wake_up();
				//deleting unused buffer
				if (buffer_unused) {
					LOG_LL_TRACE(logger_, "Core: {}, removed buffer: {:p} ({} Kb).",
						name(),
						&buffer,
						buffer.size() / 1024);
					bufferstore_.remove(buff_ind);
					buffer_num--;
				}
				else {
					buff_ind++;
				}
				//restart with full_flush (already flushed buffers can contain new logs! (lockless queue))
				if (!flush_all && sink_flush) {
					flush_all = true;
					buff_ind = 0;
					LOG_LL_TRACE(logger_, "Core: {}, Switch to full log_buffer flush.", name());
				}
			}
			else {
				buff_ind++;
			}
		}
		flush_all_buffers = flush_all;
		sink_flush_needed = sink_flush;
		LOG_LL_TRACE(logger_, "Core: {}, flushed log_buffers (all: {}).", name(), flush_all);
    }

    core_impl::steady_msec core_impl::flush_sinks(
		bool flush_all_sinks, 
		steady_msec current_time) noexcept 
	{
        auto next_flush_time = (core_impl::steady_msec::max)();
        std::lock_guard<std::mutex> ss_guard(sinkstore_mutex_);
        for (auto& sink : sinkstore_) {
            auto sink_next_flush{ sink.pimpl()->next_flush_time() };
            if (flush_all_sinks 
                || current_time >= sink_next_flush
                || sink.pimpl()->needs_immediate_flush())
            {
                LOG_LL_TRACE(logger_, "Core: {} flushing sink {:p}.", name(), sink.pimpl());
                sink.pimpl()->flush(current_time);
                sink_next_flush = sink.pimpl()->next_flush_time();
            }
            
            if (sink_next_flush < next_flush_time) {
                next_flush_time = sink_next_flush;
            }
        }
        return next_flush_time;
    }

    void core_impl::run() noexcept
    {
        LOG_LL_INFO(logger_, 
            "fstlog v{}, core: {} started a background thread, thread id: {}, polling interval: {} millisec.", 
            version(), name(), this_thread::get_id(), poll_interval().count());
        
        steady_msec current_time;
        std::uintmax_t cycle_count = 0;
        bool stopping = false;
        while (!stopping) {
            cycle_count++;
            current_time = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now());
            bool flush_all_buffers{ false };
            bool sink_flush_needed{ false };
            std::unique_lock<std::mutex> lock(core_mutex_);
            bool can_sleep{ true };
            do {
                if (data_ready_) {
					LOG_LL_TRACE(logger_, "Not sleeping, data_ready.");
					data_ready_ = false;
                    can_sleep = false;
                }
                if (current_time >= next_buffer_poll_) {
					LOG_LL_TRACE(logger_, "Not sleeping, next_buffer_poll time.");
					flush_all_buffers = true;
                    can_sleep = false;
                }
                if (current_time >= next_sink_flush_)
                {
					LOG_LL_TRACE(logger_, "Not sleeping, next_sink_flush_time.");
					sink_flush_needed = true;
                    can_sleep = false;
                }
                if (background_thread_stop_requested_) {
					LOG_LL_TRACE(logger_, "Not sleeping, stopped.");
					background_thread_stop_requested_ = false;
                    stopping = true;
                    sink_flush_needed = true;
                    can_sleep = false;
                }

                if (can_sleep) {
					auto sleep_until_time = next_buffer_poll_ < next_sink_flush_ ? next_buffer_poll_ : next_sink_flush_;
					// can not use wait_until() without clamping if time_point is max() (poll time is set to 0)
					constexpr auto clamp_dur{ std::chrono::hours{ 168 } };
					const auto clamped_time{ current_time + clamp_dur };
					if (sleep_until_time > clamped_time) {
						sleep_until_time = clamped_time;
					}
                    LOG_LL_TRACE(logger_, 
                        "Next log_buffer poll in: {} msec, sink flush in: {} msec, background thread will sleep for: {} msec.",
                        (std::chrono::duration_cast<std::chrono::milliseconds>(next_buffer_poll_ - current_time).count()),
                        (std::chrono::duration_cast<std::chrono::milliseconds>(next_sink_flush_ - current_time).count()),
                        (std::chrono::duration_cast<std::chrono::milliseconds>(sleep_until_time - current_time).count()));
                    background_thread_sleeping_ = true;
                    // does not throw (clocks, time points, and durations provided by the standard library never throw)
					background_thread_condvar_.wait_until(lock, sleep_until_time);
					background_thread_sleeping_ = false;
                    current_time = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now());
                    LOG_LL_TRACE(logger_, 
                        "Background thread woke up {} msec early.",
                        std::chrono::duration_cast<std::chrono::milliseconds>(sleep_until_time - current_time).count());
                }
            } while (can_sleep);
            lock.unlock();
            
            read_buffers(flush_all_buffers, sink_flush_needed);
            //flushing sinks
            if (sink_flush_needed) {
                auto new_flush_time = flush_sinks(stopping, current_time);
                LOG_LL_TRACE(logger_, "Setting next_sink_flush_ to take place in : {} millisecs.",
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        new_flush_time - current_time).count());
                lock.lock();
                next_sink_flush_ = new_flush_time;
            }
            //updating next log_buffer_poll time
            if (flush_all_buffers && buffer_poll_interval_ != std::chrono::milliseconds{0}) {
                if (!sink_flush_needed) lock.lock();
                next_buffer_poll_ = current_time + buffer_poll_interval_;
                auto temp_time = next_buffer_poll_;
                lock.unlock();
                LOG_LL_TRACE(logger_, "Setting next_buffer_poll_ to take place in : {} millisecs.",
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        temp_time - current_time).count());
                
            }
        }
                
        if constexpr (ut_cast(level::FSTLOG_COMPILETIME_LOGLEVEL) >= ut_cast(level::Debug)) {
            LOG_LL_DEBUG(logger_, "Core: {}, dropped {} self log messages.",
                name(), logger_.dropped());
            LOG_LL_DEBUG(logger_, "fstlog: v{}, core: {} stopping, background thread main loop run {} times.", 
				version(), name(), cycle_count);
            //all buffers and sinks are flushed (flushing again ensuring these logs are written to sinks)
            bool flush_all_buffers{ true };
            bool sink_flush_needed{ true };
            read_buffers(flush_all_buffers, sink_flush_needed);
            flush_sinks(true, current_time);
        }
    }

	std::string_view core_impl::version() noexcept {
		return FSTLOG_VERSION;
	}
	int core_impl::version_major() noexcept {
		return FSTLOG_VERSION_MAJOR;
	}
	int core_impl::version_minor() noexcept {
		return FSTLOG_VERSION_MINOR;
	}
	int core_impl::version_patch() noexcept {
		return FSTLOG_VERSION_PATCH;
	}

	void core_impl::add_reference() noexcept {
		// std::uintptr_t can not overflow if add_reference() is called from a new object only once,
		// a new object is at least std::uintptr_t size 
		// and memory would be exhausted before overflow could occur 
		reference_counter_.fetch_add(1, std::memory_order_relaxed);
	}

	// returns true if this was the last reference
	bool core_impl::remove_reference() noexcept {
		const auto prev_count =
			reference_counter_.fetch_sub(1, std::memory_order_acq_rel);
		FSTLOG_ASSERT(prev_count != 0);
		return prev_count == 1;
	}
}
//if mutex.lock() fails and throws program crashes
//if unique_lock.lock() fails and throws program crashes
