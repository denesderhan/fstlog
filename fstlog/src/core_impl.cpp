//Copyright © 2022-present, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <core_impl.hpp>

#include <cassert>
#include <cstdlib>
#include <type_traits>
#include <utility>

#include <detail/deadline_helper.hpp>
#include <detail/log_buffer_impl.hpp>
#include <detail/log_buffer_unread_data.hpp>
#include <detail/make_allocated.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/log_buffer.hpp>
#include <fstlog/sink/sink.hpp>
#include <fstlog/version.hpp>
#include <logger/log_macro_background.hpp>
#include <sink/sink_interface.hpp>

namespace fstlog {
    core_impl::~core_impl() noexcept {
        if (id_ == 0) return; // uninitialized
        // running() check prevents use of logger_ in stop()
        if (running()) {
            stop();
        }
        else {
            flush();
        }
        // release all sinks
        {
            std::lock_guard<std::mutex> s_guard(sinkstore_mutex_);
            for (auto& sink : sinkstore_) {
                // no flush needed, after stop() the run() thread flushed all before exiting
                sink.pimpl()->set_next_flush_time(std::nullopt);
                sink.pimpl()->release();
            }
        }
        
        // release TLS buffer index
        std::lock_guard<std::mutex> grd(init_mutex_);
        FSTLOG_ASSERT(tls_buffer_index_ >= 0 && static_cast<std::size_t>(tls_buffer_index_) < tls_buffer_index_used_.size());
        if (tls_buffer_index_ >= 0 && static_cast<std::size_t>(tls_buffer_index_) < tls_buffer_index_used_.size()) {
            tls_buffer_index_used_[tls_buffer_index_] = false;
        }
        tls_buffer_index_ = -1;
        id_ = 0;
    }

    // init() is thread-safe only to other init()-s, 
    // multi-threaded use of the object is allowed only after a successfull init()! 
    [[nodiscard]] error_code core_impl::init(std::string_view name, memory_resource* resource) noexcept {
        std::lock_guard<std::mutex> grd(init_mutex_);
        if (resource == nullptr) {
            return error_code::mem_res_null;
        }
        if (id_ != 0) {
            return error_code::double_init;
        }
        if (next_id_ == (std::numeric_limits<decltype(next_id_)>::max)()) {
            return error_code::obj_limit;
        }
        std::size_t index = 0;
        for (; index < tls_buffer_index_used_.size(); index++) {
            if (!tls_buffer_index_used_[index]) {
                tls_buffer_index_used_[index] = true;
                tls_buffer_index_ = static_cast<int>(index);
                id_ = next_id_++;
                break;
            }
        }
        // if we can not get a tls_buffer_index_
        if (id_ == 0) {
            return error_code::core_limit;
        }
        
        memory_resource_ = resource;
        name_ = name;
        // initialize to default here (no include for detail::normalized_interval() in header)
        buffer_poll_interval_ = detail::normalized_interval(config::default_polling_interval);
        next_buffer_poll_ = detail::next_deadline(timer_clock::now(), buffer_poll_interval_);
        bufferstore_ = dyn_array<log_buffer>(resource);
        sinkstore_ = dyn_array<sink>(resource);
        
        error_code err = error_code::none;
        if (!bufferstore_.grow(32)) err = error_code::alloc_fail;
        if (err == error_code::none) {
            if (!sinkstore_.grow(8)) err = error_code::alloc_fail;
        }
        // if there is no self logging, no need to init self buffer
        if constexpr (level::FSTLOG_COMPILETIME_LOGLEVEL != level::None) {
            if (err == error_code::none) {
#ifdef FSTLOG_DEBUG
                auto log_buff = get_buffer(128 * 1024);
#else
                auto log_buff = get_buffer(4 * 1024);
#endif // FSTLOG_DEBUG
                if (!log_buff.good()) err = error_code::alloc_fail;
                if (err == error_code::none) {
                    logger_.init(log_buff);
                }
            }
        }
        
        if (err == error_code::none) {
            // always try to start the background thread
            start();
            if (!running()) err = error_code::thread_fail;
        }
        
        if (err != error_code::none) {
            // cleanup on fail (without it alive but failed objects can block all indexes)
            tls_buffer_index_used_[tls_buffer_index_] = false;
            tls_buffer_index_ = -1;
            id_ = 0;
            // we dont do next_id_--; burning the id_ is safer (ids must be unique for the program lifetime)

            memory_resource_ = nullptr;
            name_ = small_string<32>{};

            buffer_poll_interval_ = std::nullopt;
            next_buffer_poll_ = std::nullopt;
            // deallocations:
            bufferstore_ = dyn_array<log_buffer>{};
            sinkstore_ = dyn_array<sink>{};
            logger_.init(log_buffer{});
        }
        return err;
    }

    [[nodiscard]] bool core_impl::running() const noexcept {
        FSTLOG_ASSERT(id_ != 0 && "Uninitialized use of core_impl!");
        if (id_ == 0) return false; // uninitialized
        std::lock_guard<std::mutex> s_guard(background_thread_mutex_);
        return background_thread_.joinable();
    }

    void core_impl::start() noexcept
    {
        FSTLOG_ASSERT(id_ != 0);
        if (id_ == 0) return; // uninitialized
        std::lock_guard<std::mutex> s_guard(background_thread_mutex_);

        if (background_thread_.joinable()) {
            LOG_LL_WARN(logger_, "start() failed, background thread already running.");
            return;
        }
                        
        {
            //locking is unnecessary (thread is not running) but does not hurt
            std::lock_guard<std::mutex> c_guard(core_mutex_);
            stop_requested_ = false;
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
        FSTLOG_ASSERT(id_ != 0 && "Uninitialized use of core_impl!");
        if (id_ == 0) return;
        std::lock_guard<std::mutex> s_guard(background_thread_mutex_);
        assert(background_thread_.get_id() != std::this_thread::get_id() && "Background thread tried to stop itself!");
        if (background_thread_.get_id() == std::this_thread::get_id()) {
            std::abort();
        }

        if (!background_thread_.joinable()) {
            LOG_LL_WARN(logger_, "stop() failed, background thread already stopped.");
            return;
        }

        {
            std::lock_guard<std::mutex> c_guard(core_mutex_);
            stop_requested_ = true;
        }
        background_thread_condvar_.notify_one();
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
    [[nodiscard]] std::chrono::milliseconds core_impl::poll_interval() const noexcept {
        FSTLOG_ASSERT(id_ != 0 && "Uninitialized use of core_impl!");
        if (id_ == 0) return std::chrono::milliseconds{ 0 };
        std::lock_guard<std::mutex> c_guard(core_mutex_);
        if (buffer_poll_interval_) return *buffer_poll_interval_;
        return std::chrono::milliseconds(0);
    }

    // Sets poll interval for log_buffers, returns current value.
    std::chrono::milliseconds core_impl::poll_interval(std::chrono::milliseconds interval) noexcept {
        FSTLOG_ASSERT(id_ != 0 && "Uninitialized use of core_impl!");
        if (id_ == 0) return std::chrono::milliseconds{ 0 };
        std::chrono::milliseconds prev_interval{ 0 };
        std::optional<std::chrono::milliseconds> set_interval;
        {
            std::lock_guard<std::mutex> c_guard(core_mutex_);
            if (buffer_poll_interval_) {
                prev_interval = *buffer_poll_interval_;
            }
            buffer_poll_interval_ = detail::normalized_interval(interval);
            next_buffer_poll_ = detail::next_deadline(timer_clock::now(), buffer_poll_interval_);
            set_interval = buffer_poll_interval_;
        }
        background_thread_condvar_.notify_one();
        if (set_interval) {
            LOG_LL_INFO(logger_, "Log buffer polling interval set to: {} milliseconds (requested: {} milliseconds).", 
                set_interval->count(), interval.count());
        }
        else {
            LOG_LL_INFO(logger_, "Log buffer polling switched off (requested: {} milliseconds).", interval.count());
        }
        return prev_interval;
    }

    [[nodiscard]] bool core_impl::add_sink(sink new_sink) noexcept {
        FSTLOG_ASSERT(id_ != 0 && "Uninitialized use of core_impl!");
        if (id_ == 0) return false;
        auto* const sink_ptr = new_sink.pimpl();
        if (sink_ptr == nullptr) {
            LOG_LL_WARN(logger_,
                "Failed to add sink, sink was nullptr!");
            return false;
        }

        std::optional<timer_point> sink_flush_time{ std::nullopt };
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
                    "Failed to add sink: {:p}, sink is already used by this core!", sink_ptr);
                return false;
            }
            if (!sink_ptr->use()) {
                LOG_LL_WARN(logger_,
                    "Failed to add sink: {:p}, sink is used by another core!", sink_ptr);
                return false;
            }
            if (!sinkstore_.try_push_back(std::move(new_sink))) {
                sink_ptr->release();
                LOG_LL_ERROR(logger_,
                    "Failed to add sink: {:p}, memory allocation failed!", sink_ptr);
                return false;
            }

            sink_flush_time = detail::next_deadline(timer_clock::now(), sink_ptr->flush_interval());
            sink_ptr->set_next_flush_time(sink_flush_time);
        }
        
        bool sink_flush_changed = false;
        if (sink_flush_time) {
            std::lock_guard<std::mutex> c_guard(core_mutex_);
            if (!next_sink_flush_ || sink_flush_time < next_sink_flush_) {
                next_sink_flush_ = sink_flush_time;
                sink_flush_changed = true;
            }
        }
        if (sink_flush_changed) { // sink_flush_time != std::nullopt
            background_thread_condvar_.notify_one();
            LOG_LL_TRACE(logger_, "Changing next_sink_flush_ to: {} milliseconds from now.",
                std::chrono::duration_cast<std::chrono::milliseconds>(*sink_flush_time - timer_clock::now()).count());
        }
        LOG_LL_DEBUG(logger_, "Added sink: {:p}.", sink_ptr);
        return true;
    }

    // Removes sink from core, sink will miss all unflushed logs.
    // Calling core.flush() or core.stop() prior to, minimizes log loss.
    // To prevent log loss, add a new sink (with same filter), call flush() then release old sink.
    [[nodiscard]] bool core_impl::release_sink(sink_interface* sink_ptr) noexcept {
        FSTLOG_ASSERT(id_ != 0);
        if (id_ == 0) return false; // uninitialized
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
            sink_to_release.pimpl()->flush();
            sink_to_release.pimpl()->set_next_flush_time(std::nullopt);
            sink_to_release.pimpl()->release(); // Must be called last! (set_next_flush_time() would clobber another add_sink())
            LOG_LL_DEBUG(logger_, "Released sink: {:p}.", sink_ptr);
            return true;
        }
        else {
            LOG_LL_WARN(logger_, "Failed to release sink: {:p}, not used by this core!", sink_ptr);
            return false;
        }
    }

    void core_impl::flush() noexcept {
        FSTLOG_ASSERT(id_ != 0);
        if (id_ == 0) return; // uninitialized
        bool flush_all_buffers{ true };
        bool sink_flush_needed{ true };
        read_buffers(flush_all_buffers, sink_flush_needed);
        
        // Do not touch next_sink_flush_ here: flush_sinks(true) updates each sink's
        // own next_flush_time(), so the old next_sink_flush_ stays a safe lower bound
        // and run() will reconcile it lazily on its next wakeup without flushing early.
        [[maybe_unused]] const auto new_sink_flush = flush_sinks(true);
    }

    [[nodiscard]] log_buffer core_impl::get_buffer(std::uint32_t buffer_size) noexcept {
        FSTLOG_ASSERT(id_ != 0 && "Uninitialized use of core_impl!");
        if (id_ == 0) return log_buffer{};
        const auto resource = get_memory_resource();
        log_buffer out{ make_allocated<log_buffer_impl>(resource, buffer_size, resource) };
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
        FSTLOG_ASSERT(id_ != 0 && "Uninitialized use of core_impl!");
        log_buffer_unread_data to_read;
        buffer.get_unread(to_read);
        if (to_read.size1 == 0) return;
        FSTLOG_ASSERT(to_read.pos1 != nullptr);
        std::lock_guard<std::mutex> ss_guard(sinkstore_mutex_);
        for (auto& s : sinkstore_) {
            [[maybe_unused]] const auto err1 = s.pimpl()->sink_msg_block(to_read.pos1, to_read.size1);
            if constexpr (ut_cast(level::FSTLOG_COMPILETIME_LOGLEVEL) >= ut_cast(level::Error)) {
                if (err1 == error_code::none) {
                    LOG_LL_TRACE(logger_,
                        "Reading ({:4} Kb) buffer({:p}): {:6} bytes at {:p} to sink: {:p}",
                        buffer.size() / 1024, &buffer,
                        to_read.size1, to_read.pos1, s.pimpl());
                }
                else {
                    LOG_LL_ERROR(logger_,
                        "Failed reading ({:4} Kb) buffer({:p}): {:6} bytes at {:p} to sink: {:p}",
                        buffer.size() / 1024, &buffer, 
                        to_read.size1, to_read.pos1, s.pimpl());
                    LOG_LL_ERROR(logger_, "{}", std::string_view(error_message(err1)));
                }
            }
            if (to_read.size2 != 0) {
                FSTLOG_ASSERT(to_read.pos2 != nullptr);
                [[maybe_unused]] const auto err2 = s.pimpl()->sink_msg_block(to_read.pos2, to_read.size2);
                if constexpr (ut_cast(level::FSTLOG_COMPILETIME_LOGLEVEL) >= ut_cast(level::Error)) {
                    if (err2 == error_code::none) {
                        LOG_LL_TRACE(logger_,
                            "Reading ({:4} Kb) buffer({:p}): {:6} bytes at {:p} to sink: {:p}",
                            buffer.size() / 1024, &buffer,
                            to_read.size2, to_read.pos2, s.pimpl());
                    }
                    else {
                        LOG_LL_ERROR(logger_,
                            "Failed reading ({:4} Kb) buffer({:p}): {:6} bytes at {:p} to sink: {:p}",
                            buffer.size() / 1024, &buffer,
                            to_read.size2, to_read.pos2, s.pimpl());
                        LOG_LL_ERROR(logger_, "{}", std::string_view(error_message(err2)));
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
        FSTLOG_ASSERT(id_ != 0 && "Uninitialized use of core_impl!");
        bool flush_all = flush_all_buffers || sink_flush_needed;
        bool sink_flush = sink_flush_needed;
        std::lock_guard<std::mutex> bs_guard(bufferstore_mutex_);
        auto buffer_num{ bufferstore_.size() };
        
        bool restart_with_full{ false };
        do {
            restart_with_full = false;
            // take a snapshot of the current write positions
            // this is needed only to minimize unordered messages in the output sink(s)
            // reading the write_pos interleaved with data reading will cause more out of order messages
            // the snapshot can not guarantee that all messages will be ordered
            for (std::size_t buff_ind = 0; buff_ind < buffer_num; buff_ind++) {
                bufferstore_[buff_ind].pimpl()->snapshot_write_pos();
            }

            std::size_t buff_ind{ 0 };
            while (buff_ind < buffer_num) {
                log_buffer_impl& buffer = *bufferstore_[buff_ind].pimpl();
                const bool buffer_unused = buffer.use_count() == 1;
                bool flushing = flush_all || buffer.flush_requested()
                    || buffer_unused || buffer.half_full();
                if (flushing) {
                    read_buffer(buffer, sink_flush);
                    // we always wake the producer, it may make progress after a partial read too 
                    // (if not it will just do an other request cycle)
                    if (buffer.flush_requested()) buffer.wake_producer();
                    // deleting unused buffer ( empty() checks the  the live atomic write position not the snapshot )
                    if (buffer_unused && buffer.empty()) {
                        LOG_LL_DEBUG(logger_, "Removing unused buffer: {:p} ({} Kb).",
                            &buffer, buffer.size() / 1024);
                        // remove does swap + pop (iterators invalidated)
                        bufferstore_.remove(buff_ind);
                        buffer_num--;
                    }
                    else {
                        buff_ind++;
                    }
                    //restart with full_flush (left out buffers can contain new logs!)
                    if (!flush_all && sink_flush) {
                        LOG_LL_TRACE(logger_, "Sink full during log buffer read forced re-read of all loging buffers.");
                        flush_all = true;
                        restart_with_full = true;
                        break;
                    }
                }
                else {
                    buff_ind++;
                }
            }
        } while (restart_with_full);
        flush_all_buffers = flush_all;
        sink_flush_needed = sink_flush;
        LOG_LL_TRACE(logger_, "Log buffer read finished{}", std::string_view(flush_all ? ", all buffers read." : "."));
    }

    [[nodiscard]] std::optional<core_impl::timer_point> core_impl::flush_sinks(
        bool flush_all_sinks) noexcept
    {
        FSTLOG_ASSERT(id_ != 0 && "Uninitialized use of core_impl!");
        LOG_LL_TRACE(logger_, "Flushing{}sinks.", std::string_view(flush_all_sinks ? " all " : " selected "));
        std::optional<timer_point> next_flush_time{ std::nullopt };
        std::lock_guard<std::mutex> ss_guard(sinkstore_mutex_);
        for (auto& sink : sinkstore_) {
            auto sink_next_flush{ sink.pimpl()->next_flush_time() };
            auto now = timer_clock::now();
            bool do_flush{ false };
            std::string_view flush_cause{ "none" }; // trivially stripped by the optimizer if not used
            // short circuiting check
            if (flush_all_sinks) {
                do_flush = true;
                flush_cause = "all";
            }
            else if (sink_next_flush && now >= *sink_next_flush) {
                do_flush = true;
                flush_cause = "timer";
            }
            else if (sink.pimpl()->needs_immediate_flush()) {
                do_flush = true;
                flush_cause = "full";
            }

            if constexpr (ut_cast(level::FSTLOG_COMPILETIME_LOGLEVEL) >= ut_cast(level::Trace)) {
                if (sink_next_flush) {
                    LOG_LL_TRACE(logger_, "Sink {:p}, flush timer expires {} milliseconds from now.", sink.pimpl(),
                        std::chrono::duration_cast<std::chrono::milliseconds>(*sink_next_flush - timer_clock::now()).count());
                }
                else {
                    LOG_LL_TRACE(logger_, "Sink {:p}, flush timer is OFF.", sink.pimpl());
                }
                if (do_flush) {
                    LOG_LL_TRACE(logger_, "Sink {:p}, FLUSHING, cause: {}", sink.pimpl(), flush_cause);
                }
                else {
                    LOG_LL_TRACE(logger_, "Sink {:p}, NOT FLUSHING.", sink.pimpl());
                }
            }
            
            if (do_flush) {
                sink.pimpl()->flush();
                now = timer_clock::now();
                const auto new_sink_next_flush = detail::next_deadline(now, sink.pimpl()->flush_interval());
                if(new_sink_next_flush != sink_next_flush) {
                    sink_next_flush = new_sink_next_flush;
                    sink.pimpl()->set_next_flush_time(sink_next_flush);
                }
            }
            if constexpr (ut_cast(level::FSTLOG_COMPILETIME_LOGLEVEL) >= ut_cast(level::Trace)) {
                if (sink_next_flush) {
                    LOG_LL_TRACE(logger_, "Sink {:p}, setting flush timer to {} milliseconds from now.", sink.pimpl(),
                        std::chrono::duration_cast<std::chrono::milliseconds>(*sink_next_flush - timer_clock::now()).count());
                }
                else {
                    LOG_LL_TRACE(logger_, "Sink {:p}, setting flush timer to OFF.", sink.pimpl());
                }
            }
            if (!sink_next_flush) continue;
            if (!next_flush_time || *sink_next_flush < *next_flush_time) {
                next_flush_time = sink_next_flush;
            }
        }
        return next_flush_time;
    }

    core_impl::pending_tasks core_impl::take_pending(timer_point now) noexcept {
        pending_tasks tasks;
        if (data_ready_) {
            LOG_LL_TRACE(logger_, "Pending task: data_ready_.");
            data_ready_ = false;
            tasks.data_ready = true;
        }
        if (next_buffer_poll_ && now >= *next_buffer_poll_) {
            LOG_LL_TRACE(logger_, "Pending task: next_buffer_poll_.");
            tasks.poll_all = true;
        }
        if (next_sink_flush_ && now >= *next_sink_flush_) {
            LOG_LL_TRACE(logger_, "Pending task: next_sink_flush_.");
            next_sink_flush_ = std::nullopt; // this could be left out here, in run it is set anyways
            tasks.flush_sinks = true;
        }
        if (stop_requested_) {
            LOG_LL_TRACE(logger_, "Pending task: stop_requested_.");
            stop_requested_ = false;
            tasks.stop = true;
            tasks.flush_sinks = true;
        }
        return tasks;
    }

    core_impl::timer_point core_impl::sleep(timer_point now, std::unique_lock<std::mutex> &lock) noexcept {
        if constexpr (ut_cast(level::FSTLOG_COMPILETIME_LOGLEVEL) >= ut_cast(level::Trace)) {
            if (next_buffer_poll_) {
                LOG_LL_TRACE(logger_, "next_buffer_poll_ time is: {} milliseconds from now",
                    (std::chrono::duration_cast<std::chrono::milliseconds>(*next_buffer_poll_ - timer_clock::now()).count()));
            }
            else {
                LOG_LL_TRACE(logger_, "next_buffer_poll_ is std::nullopt");
            }
            if (next_sink_flush_) {
                LOG_LL_TRACE(logger_, "next_sink_flush_ time is: {} milliseconds from now",
                    (std::chrono::duration_cast<std::chrono::milliseconds>(*next_sink_flush_ - timer_clock::now()).count()));
            }
            else {
                LOG_LL_TRACE(logger_, "next_sink_flush_ is std::nullopt");
            }
        }

        // we can use next_sink_flush_, it was not consumed (tasks.empty() == true)
        // min(next_buffer_poll_, next_sink_flush_)
        std::optional<timer_point> next_wakeup{ next_buffer_poll_ };
        if (!next_wakeup ||
            (next_sink_flush_ && *next_sink_flush_ < *next_wakeup)) {
            next_wakeup = next_sink_flush_; // next_sink_flush_ was not consumed
        }

        // sleep without timer
        if (!next_wakeup) {
            LOG_LL_TRACE(logger_, "No timers set, background thread will sleep until woken up.");
            // does not throw (clocks, time points, and durations provided by the standard library never throw)
            const auto prev_now = now;
            background_thread_condvar_.wait(lock);
            now = timer_clock::now();
            LOG_LL_TRACE(logger_, "Background thread slept {} milliseconds.",
                std::chrono::duration_cast<std::chrono::milliseconds>(now - prev_now).count());
        }
        // sleep with timer
        else {
            LOG_LL_TRACE(logger_, "Background thread will sleep for: {} milliseconds.",
                (std::chrono::duration_cast<std::chrono::milliseconds>(*next_wakeup - timer_clock::now()).count()));
            // does not throw (clocks, time points, and durations provided by the standard library never throw)
            background_thread_condvar_.wait_until(lock, *next_wakeup);
            now = timer_clock::now();
            const auto delta = *next_wakeup - now;
            LOG_LL_TRACE(logger_, "Background thread woke up {} milliseconds {}.",
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::abs(delta)).count(),
                std::string_view(*next_wakeup >= now ? "early" : "late"));

        }
        return now;
    }

    void core_impl::run() noexcept
    {
        std::unique_lock<std::mutex> lock(core_mutex_);
        FSTLOG_ASSERT(id_ != 0 && "Uninitialized use of core_impl!");
        FSTLOG_ASSERT((std::is_same_v<decltype(version()), const char*>) && "Dangling pointer danger!");
        LOG_LL_INFO(logger_,
            "fstlog v{}, core: {} started a background thread, thread id: {}, polling interval: {} millisec.",
            std::string_view(version()), name_, fstlog::this_thread::get_id(), 
            buffer_poll_interval_ ? (*buffer_poll_interval_).count() : 0);
        std::uintmax_t cycle_count = 0;
        pending_tasks tasks{};
        while (!tasks.stop) {
            timer_point now{ timer_clock::now() };
            cycle_count++;
            LOG_LL_TRACE(logger_, "Main loop start, cycle: {}", cycle_count);
            tasks = take_pending(now);
            // if no tasks, the thread will sleep
            if (tasks.empty()) {
                // take_panding did not consume the next_sink_flush_ value
                // we can use it in sleep()
                now = sleep(now, lock);
                // updating tasks after wake-up
                // all members of tasks are false before the assignment
                // so we are not clobbering any values
                tasks = take_pending(now);
            }

            if (tasks.stop) {
                LOG_LL_DEBUG(logger_, "Logger: {}, dropped {} (lifetime) log messages.",
                    logger_.name(), logger_.dropped());
                LOG_LL_DEBUG(logger_, "fstlog: v{}, core: {} stopping, background thread main loop run {} times.",
                    std::string_view(version()), name(), cycle_count);
            }

            // Reading log buffers to sinks, we do a read after all wake-ups. 
            // Producer threads will call wakeup() if their buffer is half full.  
            // If the consumer (this run()) misses it (not in sleep),
            // the producer will call notify_data_ready() when it runs out of space.
            // Emptying the buffer on a wakeup() is not guaranteed
            // but on a notify_data_ready() it is
            lock.unlock();
            // read_buffers will set poll_all to true, if it had to do a full buffer flush 
            // and flush_sinks to true if the sink flush task
            // must be added to the current cycle
            read_buffers(tasks.poll_all, tasks.flush_sinks);
            lock.lock();
                        
            // update buffer polling timer using the time 
            // before the call to read_buffers(), to mitigate drift
            if (tasks.poll_all) {
                next_buffer_poll_ = detail::next_deadline(now, buffer_poll_interval_);
                if constexpr (ut_cast(level::FSTLOG_COMPILETIME_LOGLEVEL) >= ut_cast(level::Trace)) {
                    if (next_buffer_poll_) {
                        LOG_LL_TRACE(logger_, "Setting next_buffer_poll_ to {} milliseconds from now.",
                            std::chrono::duration_cast<std::chrono::milliseconds>(
                                *next_buffer_poll_ - timer_clock::now()).count());
                    }
                    else {
                        LOG_LL_TRACE(logger_, "Setting next_buffer_poll_ to std::nullopt.");
                    }
                }
            }
            
            // flushing sinks, (flush_sink() calls timer_clock::now() inside)
            if (tasks.flush_sinks) {
                next_sink_flush_ = std::nullopt;
                lock.unlock();
                std::optional<timer_point> new_sink_timer = flush_sinks(tasks.stop);
                lock.lock();
                
                // updating next_sink_flush_ timer, we have set it to nullopt above, but a
                // concurrent call to add_sink() during unlocked mutex could have changed its value!
                // We must compare it again and use the earlier one.
                if (new_sink_timer &&  
                    (!next_sink_flush_ || *new_sink_timer < *next_sink_flush_))
                {
                    next_sink_flush_ = new_sink_timer;
                    LOG_LL_TRACE(logger_, "Setting next_sink_flush_ to {} milliseconds from now.",
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            *new_sink_timer - timer_clock::now()).count());
                }
            }
        }
    }

    // calling this after a failed init() aborts the process
    [[nodiscard]] log_buffer& core_impl::tls_buffer() noexcept {
        FSTLOG_ASSERT(id_ != 0 && "Uninitialized use of core_impl!");
        FSTLOG_ASSERT(tls_buffer_index_ >= 0
            && static_cast<std::size_t>(tls_buffer_index_) < tls_buffers_.size());
        if (tls_buffer_index_ < 0 || static_cast<std::size_t>(tls_buffer_index_) >= tls_buffers_.size()) std::abort();
        log_buffer& buffer = tls_buffers_[tls_buffer_index_].second;
        auto& stored_id = tls_buffers_[tls_buffer_index_].first;
        // if this is the first access, we clear the buffer slot before returning it, 
        // initialization is the callers task.
        if (id_ != stored_id) {
            buffer = log_buffer{};
            stored_id = id_;
        }
        return buffer;
    }

    void core_impl::add_reference() noexcept {
        FSTLOG_ASSERT(id_ != 0 && "Uninitialized use of core_impl!");
        // std::uintptr_t can not overflow if add_reference() is called from a new object only once,
        // a new object is at least std::uintptr_t size 
        // and memory would be exhausted before overflow could occur 
        reference_counter_.fetch_add(1, std::memory_order_relaxed);
    }

    // returns true if this was the last reference
    bool core_impl::remove_reference() noexcept {
        FSTLOG_ASSERT(id_ != 0 && "Uninitialized use of core_impl!");
        const auto prev_count =
            reference_counter_.fetch_sub(1, std::memory_order_acq_rel);
        FSTLOG_ASSERT(prev_count != 0);
        return prev_count == 1;
    }
} // namespace fstlog
// if mutex.lock() fails and throws program crashes
// if unique_lock.lock() fails and throws program crashes
