//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>

#include <fstlog/core.hpp>
#include <fstlog/detail/api_def.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/fstlog_allocator.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/internal_msg_header.hpp>
#include <fstlog/detail/types.hpp>
#include <fstlog/detail/ut_cast.hpp>

namespace fstlog {
	class log_buffer;
	struct log_buffer_unread_data;
	class alignas(constants::cache_ls_nosharing) log_buffer_impl {
	public:
		using allocator_type = fstlog_allocator;

		using wrapper_type = log_buffer;

		log_buffer_impl(std::uint32_t buffer_size, allocator_type const& alloc = {}) noexcept;
		log_buffer_impl(const log_buffer_impl&) = delete;
		log_buffer_impl(log_buffer_impl&&) = delete;
		log_buffer_impl& operator=(const log_buffer_impl&) = delete;
		log_buffer_impl& operator=(log_buffer_impl&&) = delete;
		~log_buffer_impl() noexcept;
	
		FSTLOG_API bool good() const noexcept;
		
		auto size() const noexcept {
			return buffer_size_;
		}
		auto max_message_size() const noexcept {
			return max_message_size_;
		}
		//called only by producer
		auto writeable_size() const noexcept {
			return writeable_size_;
		}
		//called only by producer
		unsigned char* write_ptr() const noexcept {
			return begin_ + (write_pos() & buffer_mask_);
		}
		//called only by producer
		void advance_write_pos(std::uint32_t adv_size) noexcept {
			FSTLOG_ASSERT(adv_size <= writeable_size_);
			writeable_size_ -= adv_size;
			[[maybe_unused]] const auto prev_pos = write_pos_.fetch_add(adv_size, std::memory_order_release);
			FSTLOG_ASSERT((prev_pos & buffer_mask_) <= size() - adv_size);
		}
		//called only by consumer
		void get_unread(log_buffer_unread_data& unread) noexcept;
		//called only by consumer
		void advance_read_pos(std::uint32_t size) noexcept;
		
		bool half_full() const noexcept {
			return (write_pos() - read_pos()) > half_buffer_mask_;
		}

		//called only by producer
		void request_flush_if_needed(
			std::uint32_t prev_write_pos,
			core const& core) noexcept
		{
			if ((prev_write_pos ^ write_pos()) <= quarter_buffer_mask_) {
				//no flush needed
			}
			else if (half_full()) {
				flush_requested_.store(true, std::memory_order_release);
				core.notify_data_ready();
			}
		}

		//called only by producer
		void update_producer_state(std::uint32_t needed_size) noexcept {
			FSTLOG_ASSERT(needed_size <= buffer_size_);

			const auto write_p = write_pos();
			const auto read_p = read_pos();

			FSTLOG_ASSERT((write_p - read_p) <= buffer_size_);
			FSTLOG_ASSERT((write_p & buffer_mask_) < buffer_size_);

			//size_until_end always >0 <=1024 (!=0)
			const std::uint32_t size_until_end = buffer_size_ - (write_p & buffer_mask_);
			//all_free >=0 <=1024
			const std::uint32_t all_free = buffer_size_ - (write_p - read_p);

			if (size_until_end <= all_free) {
				if (size_until_end >= needed_size) {
					writeable_size_ = size_until_end;
				}
				//wrap to begin (writing end of data marker to buffer)
				else {
					// write_p cant be at end of buffer because end of buffer is the beginning of buffer 
					// always will be a free byte at write_p (size_until_end >0 --> all_free > 0) 
					// free space always begins at write_p
					const auto end_marker_ptr{ begin_ + (write_p & buffer_mask_) };
					FSTLOG_ASSERT(end_marker_ptr > begin_ && end_marker_ptr < begin_ + buffer_size_);
					*end_marker_ptr = ut_cast(log_msg_type::Invalid);
					write_pos_.fetch_add(size_until_end, std::memory_order_release);
					writeable_size_ = all_free - size_until_end;
				}
			}
			else {
				writeable_size_ = all_free;
			}

			FSTLOG_ASSERT(writeable_size_ <= buffer_size_);
			FSTLOG_ASSERT(
				((read_p & buffer_mask_) <= (write_pos() & buffer_mask_)
					&& ((write_pos() & buffer_mask_) == buffer_size_ - writeable_size_))
				|| ((read_p & buffer_mask_) >= (write_pos() & buffer_mask_)
					&& (read_p & buffer_mask_) - (write_pos() & buffer_mask_) == writeable_size_));
		}

		//called by consumer
		bool flush_requested() const noexcept;
		
		std::uint32_t read_pos() const noexcept {
			return read_pos_.load(std::memory_order_acquire);
		}
		std::uint32_t write_pos() const noexcept {
			return write_pos_.load(std::memory_order_acquire);
		}

		//called by producer
		FSTLOG_API void wait_for_buffer_flush(core const& core) noexcept;

		//called by consumer
		void wake_up() noexcept;

		allocator_type const& get_allocator() const noexcept {
			return allocator_;
		}

		std::size_t use_count() const noexcept {
			return reference_counter_.load(std::memory_order_relaxed);
		}
#ifndef FSTLOG_TESTING
	private:
#endif
		void add_reference() noexcept;
		// returns true if this was the last reference
		bool remove_reference() noexcept;

		//producer/consumer read (static data, initialized at buffer creation)
		unsigned char* begin_;
		std::uint32_t max_message_size_;
		std::uint32_t buffer_size_;
		std::uint32_t buffer_mask_;
		std::uint32_t half_buffer_mask_;
		std::uint32_t quarter_buffer_mask_;
			
		//producer/consumer read /write
		alignas(constants::cache_ls_nosharing) mutable std::mutex buffer_mutex_;
		alignas(constants::cache_ls_nosharing) std::condition_variable buff_cond_var_;
		std::atomic<bool> flush_requested_{ false };
		//consumer write, producer read
		std::atomic<std::uint32_t> read_pos_{ 0 };
#ifdef FSTLOG_DEBUG
		static_assert(decltype(read_pos_)::is_always_lock_free, "Type not lock_free!");
#endif
		//producer write, consumer read
		std::atomic<std::uint32_t> write_pos_{ 0 };
#ifdef FSTLOG_DEBUG
		static_assert(decltype(write_pos_)::is_always_lock_free, "Type not lock_free!");
#endif
		//producer read/write
		std::uint32_t writeable_size_;
		
		allocator_type allocator_;

		std::atomic<std::uintptr_t> reference_counter_{ 0 };

		friend class log_buffer;
	};
	
	//if mutex.lock() fails and throws program crashes
}
