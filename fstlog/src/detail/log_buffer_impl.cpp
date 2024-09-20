//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <detail/log_buffer_impl.hpp>

#include <chrono>
#include <limits>

#include <config_buffer.hpp>
#include <detail/log_buffer_unread_data.hpp>
#include <detail/nearest_pow2.hpp>
#include <detail/nothrow_allocate.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/is_pow2.hpp>

namespace fstlog {
	log_buffer_impl::log_buffer_impl(std::uint32_t buffer_size, allocator_type const& alloc) noexcept
		: max_message_size_{ config::max_internal_log_msg_size },
		allocator_{ alloc }
	{
		//buffer_size = 0 means use default
		if (buffer_size == 0) buffer_size = config::default_ringbuffer_size;
		buffer_size_ = detail::nearest_pow2<std::uint32_t>(buffer_size);
		if (buffer_size_ < config::min_ringbuffer_size) {
			buffer_size_ = config::min_ringbuffer_size;
		}
		else if (buffer_size_ > config::max_ringbuffer_size) {
			buffer_size_ = config::max_ringbuffer_size;
		}
		begin_ = static_cast<unsigned char*>(aligned_nothrow_allocate(
			allocator_, 
			buffer_size_, 
			config::buffer_alignment));
				
		//allocation failed
		if (begin_ == nullptr) {
			buffer_size_ = 0;
		}
		else {
			//For the indexes to work:
			FSTLOG_ASSERT(is_pow2(buffer_size_));
		}

		writeable_size_ = buffer_size_;
		buffer_mask_ = buffer_size_ - 1;
		half_buffer_mask_ = (buffer_size_ >> 1) - 1;
		quarter_buffer_mask_ = (buffer_size_ >> 2) - 1;
		if (buffer_size_ < max_message_size_)
			max_message_size_ = buffer_size_;
	}

	log_buffer_impl::~log_buffer_impl() noexcept {
		if (begin_ != nullptr) {
			aligned_nothrow_deallocate(
				begin_, 
				allocator_, 
				buffer_size_, 
				config::buffer_alignment);
		}
	}


	bool log_buffer_impl::good() const noexcept {
		return begin_ != nullptr;
	}

	void log_buffer_impl::get_unread(log_buffer_unread_data& unread) noexcept {
		FSTLOG_ASSERT(unread.pos1 == nullptr && unread.pos2 == nullptr
			&& unread.size1 == 0 && unread.size2 == 0);
		const std::uint32_t read_p = read_pos();
		const std::uint32_t write_p = write_pos();
		//empty
		if (write_p == read_p) return;
		const std::uint32_t read_index{ read_p & buffer_mask_ };
		const std::uint32_t write_index{ write_p & buffer_mask_ };
		unread.pos1 = begin_ + read_index;
		if (read_index < write_index) {
			unread.size1 = write_index - read_index;
		}
		else {
			unread.size1 = size() - read_index;
			unread.pos2 = begin_;
			unread.size2 = write_index;
		}
	}

	void log_buffer_impl::advance_read_pos(std::uint32_t size) noexcept {
		read_pos_.fetch_add(size, std::memory_order_release);
	}
	
	//called by consumer
	bool log_buffer_impl::flush_requested() const noexcept {
		// lock is needed (compiler can reorder 
		// setting flush_requested_ = true 
		// and notify_core() 
		// in wait_for_buffer_flush()
		// core can read flush_requested_ == false (before it is set to true)
		std::lock_guard<std::mutex> grd{ buffer_mutex_ };
		return flush_requested_.load(std::memory_order_acquire);
	}

	void log_buffer_impl::wait_for_buffer_flush(core const& core) noexcept {
		std::unique_lock<std::mutex> lock{ buffer_mutex_ };
		flush_requested_.store(true, std::memory_order_release);
		core.notify_data_ready();
		do {
#ifdef FSTLOG_DEBUG
			buff_cond_var_.wait(lock);
#else
			//failsafe
			buff_cond_var_.wait_for(lock, std::chrono::milliseconds{ 15 });
#endif
		} while (flush_requested_.load(std::memory_order_acquire));
	}

	void log_buffer_impl::wake_up() noexcept {
		std::lock_guard<std::mutex> grd{ buffer_mutex_ };
		flush_requested_.store(false, std::memory_order_release);
		//This always tries to wake logging thread (even if it is not sleeping) bad perf.?
		buff_cond_var_.notify_all();
	}

	void log_buffer_impl::add_reference() noexcept {
		// std::uintptr_t can not overflow if add_reference() is called from a new object only once,
		// a new object is at least std::uintptr_t size 
		// and memory would be exhausted before overflow could occur 
		reference_counter_.fetch_add(1, std::memory_order_relaxed);
	}

	// returns true if this was the last reference
	bool log_buffer_impl::remove_reference() noexcept {
		const auto prev_count =
			reference_counter_.fetch_sub(1, std::memory_order_acq_rel);
		FSTLOG_ASSERT(prev_count != 0);
		return prev_count == 1;
	}
	//For the indexes to work:
	//wrap around
	static_assert((std::numeric_limits<std::uint32_t>::max)() + 1 == std::uint32_t{ 0 }, "Index type does not wrap!");
	//max half can be used
	static_assert(config::max_ringbuffer_size <= (std::numeric_limits<std::uint32_t>::max)() / 2, "Max buffer size wrong!");
	
	//First byte of header must be the log_msg_type byte (it is used in sink_msg_block()!!)
	static_assert(offsetof(internal_msg_header, msg_type) == 0);
	//Asserting struct is packed
	static_assert(offsetof(internal_msg_header, version) +
		sizeof(internal_msg_header::version) ==
		internal_msg_header::unpadded_data_size);
	static_assert(internal_msg_header::padded_data_size% constants::internal_msg_data_alignment == 0);
}
