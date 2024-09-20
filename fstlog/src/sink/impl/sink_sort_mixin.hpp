//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#pragma intrinsic(memcpy)

#include <detail/byte_span.hpp>
#include <detail/dyn_array.hpp>
#include <detail/dyn_buffer.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/internal_msg_header.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
    template<class L>
    class sink_sort_mixin : public L {
	public:
		using allocator_type = typename L::allocator_type;
	
	private:
		using steady_msec = std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds>;
		struct msg_ind;
		
	public:
		sink_sort_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(sink_sort_mixin(allocator_type{})))
			: sink_sort_mixin(allocator_type{}) {}
		explicit sink_sort_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{}))
			&& noexcept(decltype(message_indices_)(allocator_type{}))
			&& noexcept(decltype(message_data_)(allocator_type{})))
            : L(allocator),
            message_indices_{ allocator },
            message_data_{ allocator } {}

        sink_sort_mixin(const sink_sort_mixin& other) = delete;
        sink_sort_mixin(sink_sort_mixin&& other) = delete;
        sink_sort_mixin& operator=(const sink_sort_mixin& rhs) = delete;
        sink_sort_mixin& operator=(sink_sort_mixin&& rhs) = delete;

        ~sink_sort_mixin() noexcept {
            flush();
        }

        void set_max_data_size(std::uint32_t bytes) noexcept {
			max_data_size_ = bytes;
			message_data_.grow((std::size_t(max_data_size_) / 144) * 128);
			message_indices_.grow(max_data_size_ / 144);
        }

        void sink_msg(buff_span_const message) noexcept {
			FSTLOG_ASSERT(message.data() != nullptr);
			FSTLOG_ASSERT(message.size_bytes() >= internal_msg_header::padded_data_size);
			FSTLOG_ASSERT(message.size_bytes() <= (std::numeric_limits<msg_counter>::max)());
            
			const auto msg_size = message.size_bytes();
            const auto msg_ptr = message.data();
            stamp_type::rep timestamp;
            memcpy(
                &timestamp,
                msg_ptr + offsetof(internal_msg_header, timestamp),
                sizeof(internal_msg_header::timestamp));

            const std::size_t msg_pos{ message_data_.size() };
			bool success = message_data_.write(msg_ptr, msg_size);
			if(success) {
				success = message_indices_.try_push_back(
                    msg_ind{ timestamp,
                        static_cast<std::uint32_t>(msg_pos),
                        static_cast<std::uint32_t>(msg_size) });
            }
            if (!success) {
                //failsafe unsorted sinking 
				//another log_buffer can contain unsinked message for this sink
                //flush data
				flush();
				L::sink_msg(message);
				message_data_.shrink((std::size_t(max_data_size_) / 144) * 128);
				message_indices_.shrink(max_data_size_ / 144);
            }
        }

		bool needs_immediate_flush() const noexcept {
			const auto data_size{
				message_data_.size()
				+ sizeof(msg_ind) * message_indices_.size() };
			if (data_size > max_data_size_) return true;

			const auto reserves{ max_data_size_ - data_size };
			if (reserves < max_data_size_ / 4) {
				return true;
			}
			else {
				return false;
			}
		}

        void flush(steady_msec current_time) noexcept
        {
            flush();
            L::flush(current_time);
        }
        
    private:
		void flush() noexcept
        {
			if (message_indices_.empty()) {
				return;
			}
			std::qsort(
				message_indices_.begin(), 
				message_indices_.size(),
				sizeof(msg_ind),
				[](const void* a, const void* b)->int { 
					const auto first{ static_cast<msg_ind const*>(a)->timestamp };
					const auto second{ static_cast<msg_ind const*>(b)->timestamp };
					if (first < second) return -1;
					else if (first > second) return 1;
					else return 0; }
			);
			
            for (const auto& mi : message_indices_) {
                buff_span_const message{ message_data_.data() + mi.msg_pos, mi.msg_size };
                L::sink_msg(message);
            }
			message_indices_.clear();
			message_data_.clear();
        }
		
        struct msg_ind {
            stamp_type::rep timestamp;
            std::uint32_t msg_pos;
            std::uint32_t msg_size;
        };
		static_assert(sizeof(stamp_type) == sizeof(stamp_type::rep));
        std::uint32_t max_data_size_{15 * 1024};
		dyn_array<msg_ind, allocator_type> message_indices_;
		dyn_buffer<allocator_type> message_data_;
	};
}
