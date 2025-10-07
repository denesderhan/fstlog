//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <type_traits>

#include <detail/dyn_array.hpp>
#include <detail/unaligned_span.hpp>
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/internal_msg_header.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
    template<class L>
    class sink_sort_mixin : public L {
    public:
        using memory_resource_type = typename L::memory_resource_type;
    
    private:
        using steady_msec = std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds>;
        struct message_locator;
        
    public:

        explicit sink_sort_mixin(memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<L, memory_resource_type*>)
            : L(resource),
            message_locators_{ resource },
            message_buffer_{ resource },
            merge_buffer_{ resource },
            sorted_block_sizes_{ resource } {
        }

        sink_sort_mixin(const sink_sort_mixin& other) = delete;
        sink_sort_mixin(sink_sort_mixin&& other) = delete;
        sink_sort_mixin& operator=(const sink_sort_mixin& rhs) = delete;
        sink_sort_mixin& operator=(sink_sort_mixin&& rhs) = delete;

        ~sink_sort_mixin() noexcept {
            flush_impl();
        }

        error_code init_sink_sort(std::uint32_t max_data_bytes) noexcept {
            if (max_data_bytes < 512) max_data_bytes = 512;
            max_data_size_ = max_data_bytes;
            if (!message_buffer_.grow(max_data_size_)) return error_code::alloc_fail;
            if (!message_locators_.grow(max_data_size_ / 64)) return error_code::alloc_fail;
            if (!merge_buffer_.grow(max_data_size_ / 64)) return error_code::alloc_fail;
            return error_code::none;
        }

        void sink_msg(byte_span_const message) noexcept {
            FSTLOG_ASSERT(message.data_bytes() != nullptr);
            FSTLOG_ASSERT(message.size_bytes() >= internal_msg_header::padded_data_size);
            FSTLOG_ASSERT(message.size_bytes() <= (std::numeric_limits<std::uint32_t>::max)());
            
            const auto msg_size = message.size_bytes();
            const auto msg_ptr = message.data_bytes();
            stamp_type timestamp;
            std::memcpy(
                &timestamp,
                msg_ptr + offsetof(internal_msg_header, timestamp),
                sizeof(internal_msg_header::timestamp));

            const std::size_t msg_pos{ message_buffer_.size() };
            // if msg_pos would overflow we cant add message to buffer (this can not happen in practice)
            bool success = msg_pos <= (std::numeric_limits<std::uint32_t>::max)();
            if(success) success = message_buffer_.write(msg_ptr, msg_size);
            const message_locator message_meta{
                timestamp,
                static_cast<std::uint32_t>(msg_pos),
                static_cast<std::uint32_t>(msg_size)
            };
            // if msg_pos did not overflow this must be also true
            FSTLOG_ASSERT(message_locators_.size() <= (std::numeric_limits<std::uint32_t>::max)());
            if(success) success = message_locators_.try_push_back(message_meta);
            if (success) {
                const bool new_block = 
                    timestamp < last_timestamp_
                    || sorted_block_sizes_.empty(); // if data gets corrupted and timestamp becomes max() -> UB!
                if (!new_block) {
                    sorted_block_sizes_.back()++;
                }
                else {
                    // try to start a new block with current message
                    success = sorted_block_sizes_.try_push_back(1);
                    if (!success) {
                        // remove locator if failed (roll back)
                        message_locators_.pop_back();
                        // we don't have to remove the data from message_buffer_, we simply don't use it
                        // removing the data would not reduce the container's reserved memory 
                    }
                }

                last_timestamp_ = timestamp;
            }

            // failure can be caused only by memory exhaustion
            if(!success) {
                // failsafe unsorted sinking (another log_buffer can contain unsinked messages)
                // we are not loosing messages, but can not guarantee ordered sinking
                // flushing message buffer
                flush_impl();
                L::sink_msg(message);
                // try to free some memory
                message_buffer_.shrink(max_data_size_);
                message_locators_.shrink(max_data_size_ / 128);
            }
        }

        bool needs_immediate_flush() const noexcept {
            return
                message_buffer_.size() >= max_data_size_
                || sorted_block_sizes_.size() >= 64;
        }

        void flush(steady_msec current_time) noexcept
        {
            flush_impl();
            L::flush(current_time);
        }
        
    private:
        void flush_impl() noexcept
        {
            std::size_t block_num = sorted_block_sizes_.size();
            while (block_num > 1) {
                std::size_t begin = 0;
                std::size_t new_block_ind = 0;
                const std::size_t pairs = block_num / 2;
                for(std::size_t i = 0; i < pairs; i++) {
                    const std::size_t block_l_size = sorted_block_sizes_[i * 2];
                    const std::size_t block_r_size = sorted_block_sizes_[i * 2 + 1];
                    FSTLOG_ASSERT(begin + block_l_size + block_r_size <= message_locators_.size());
                    inplace_merge(begin, block_l_size, block_r_size);
                    const std::size_t merged_size = block_l_size + block_r_size;
                    begin += merged_size;
                    sorted_block_sizes_[new_block_ind++] = merged_size;
                }
                // add last block if block_num was odd
                if (block_num & 1) {
                    sorted_block_sizes_[new_block_ind++] = sorted_block_sizes_[block_num - 1];
                }
                block_num = new_block_ind;
            }

            // sink messages in sorted order
            for (const auto& msg_loc : message_locators_) {
                byte_span_const message{ 
                    message_buffer_.data() + msg_loc.msg_pos,
                    msg_loc.msg_size };
                L::sink_msg(message);
            }
            message_locators_.clear();
            message_buffer_.clear();
            sorted_block_sizes_.clear();
            last_timestamp_ = (stamp_type::max)();
        }

        void inplace_merge(std::size_t start, std::size_t block_l_size, std::size_t block_r_size) noexcept {
            FSTLOG_ASSERT(block_l_size > 0 && block_r_size > 0);
            const std::size_t total_size = block_l_size + block_r_size;
            const bool move_left = block_l_size <= block_r_size;
            const std::size_t moved_block_size = move_left ? block_l_size : block_r_size;
            // try to resize temporary buffer to fit the smaller block
            merge_buffer_.resize_uninitialized(moved_block_size);
            // if resize fails, do to memory exhaustion, fall back to qsort (highly unlikely) 
            if (merge_buffer_.size() < moved_block_size) {
                std::qsort(
                    &message_locators_[start],
                    total_size,
                    sizeof(message_locator),
                    qsort_compare
                );
                return;
            }

            if (move_left) {
                // copy left block to temp buffer merge_buffer_
                std::memcpy(
                    &merge_buffer_[0],
                    &message_locators_[start],
                    block_l_size * sizeof(message_locator));
                // left block is in the temporary buffer
                constexpr std::size_t l_begin = 0;
                const std::size_t l_end = block_l_size;
                // right block stays in place
                const std::size_t r_begin = start + block_l_size;
                const std::size_t r_end = r_begin + block_r_size;
                
                // rotate blocks if first.timestamp >= last.timestamp
                if (message_locators_[start].timestamp
                    >= message_locators_[start + total_size - 1].timestamp)
                {
                    // move right block to left
                    std::memmove(
                        &message_locators_[start],
                        &message_locators_[start + block_l_size],
                        block_r_size * sizeof(message_locator));
                    // copy left block from temp buffer to right
                    std::memcpy(
                        &message_locators_[start + block_r_size],
                        &merge_buffer_[0],
                        block_l_size * sizeof(message_locator));
                    return;
                }

                // merge blocks (forward direction left -> right)
                std::size_t l_pos = l_begin;
                std::size_t r_pos = r_begin;
                std::size_t out_pos = start;
                while (true) {
                    if (l_pos == l_end) {
                        // rest of elements are in place (right block was not moved)
                        return;
                    }
                    else if (r_pos == r_end) {
                        // copy rest of left block
                        const std::size_t copy_size = l_end - l_pos;
                        if (copy_size != 0) {
                            std::memcpy(
                                &message_locators_[out_pos], 
                                &merge_buffer_[l_pos],
                                copy_size * sizeof(message_locator));
                        }
                        return;
                    }
                    else {
                        FSTLOG_ASSERT(out_pos >= start && out_pos < start + total_size);
                        FSTLOG_ASSERT(l_pos < block_l_size);
                        FSTLOG_ASSERT(r_pos >= start + block_l_size && r_pos < start + total_size);
                        if (merge_buffer_[l_pos].timestamp <= message_locators_[r_pos].timestamp) {
                            message_locators_[out_pos++] = merge_buffer_[l_pos++];
                        }
                        else {
                            message_locators_[out_pos++] = message_locators_[r_pos++];
                        }
                    }
                }
            }
            else {
                // copy moved right block to temp buffer merge_buffer_
                std::memcpy(
                    &merge_buffer_[0],
                    &message_locators_[start + block_l_size],
                    block_r_size * sizeof(message_locator));
                // right block is in the temporary buffer
                const std::size_t r_begin = block_r_size - 1;
                constexpr std::size_t r_end = (std::numeric_limits<std::size_t>::max)();
                // left block stays in place
                const std::size_t l_begin = start + block_l_size - 1;
                const std::size_t l_end = start - 1;
                
                // rotate blocks if first.timestamp >= last.timestamp
                if (message_locators_[start].timestamp
                    >= message_locators_[start + total_size - 1].timestamp)
                {
                    // move left block to right
                    std::memmove(
                        &message_locators_[start + block_r_size],
                        &message_locators_[start],
                        block_l_size * sizeof(message_locator));
                    // copy right block from temp buffer
                    std::memcpy(
                        &message_locators_[start],
                        &merge_buffer_[0],
                        block_r_size * sizeof(message_locator));
                    return;
                }

                // merge blocks (backward direction left <- right)
                std::size_t l_pos = l_begin;
                std::size_t r_pos = r_begin;
                std::size_t out_pos = start + total_size - 1;
                while (true) {
                    if (l_pos == l_end) {
                        // copy rest of right block
                        std::size_t copy_size = r_pos + 1;
                        if (copy_size != 0) {
                            // Ensure that copying `copy_size` elements backward ends exactly at `start`
                            // i.e., the output range [start, out_pos] must match the copied block from right
                            FSTLOG_ASSERT(start + copy_size == out_pos + 1);
                            std::memcpy(
                                &message_locators_[start],
                                &merge_buffer_[0],
                                copy_size * sizeof(message_locator));
                        }
                        return;
                    }
                    else if (r_pos == r_end) {
                        // rest of elements are in place (left block was not moved)
                        return;
                    }
                    else {
                        FSTLOG_ASSERT(out_pos >= start && out_pos < start + total_size);
                        FSTLOG_ASSERT(r_pos < block_r_size);
                        FSTLOG_ASSERT(l_pos >= start && l_pos < start + block_l_size);
                        // Flipped comparison to preserve ascending order during backward merge
                        if (message_locators_[l_pos].timestamp > merge_buffer_[r_pos].timestamp) {
                            message_locators_[out_pos--] = message_locators_[l_pos--];
                        }
                        else {
                            message_locators_[out_pos--] = merge_buffer_[r_pos--];
                        }
                    }
                }
            }
        }

        // function signature for qsort does not contain noexcept, but function does not throw
        static int qsort_compare(const void* a, const void* b) {
            const auto stamp_a = static_cast<message_locator const*>(a)->timestamp;
            const auto stamp_b = static_cast<message_locator const*>(b)->timestamp;
            if (stamp_a < stamp_b) return -1;
            if (stamp_a > stamp_b) return 1;
            return 0;
        }
        
        struct message_locator {
            stamp_type timestamp;
            std::uint32_t msg_pos;
            std::uint32_t msg_size;
        };
        std::uint32_t max_data_size_{15 * 1024};
        dyn_array<message_locator> message_locators_;
        dyn_array<message_locator> merge_buffer_;
        dyn_array<std::size_t> sorted_block_sizes_;
        dyn_array<unsigned char> message_buffer_;
        stamp_type last_timestamp_{ (stamp_type::max)() };

        static_assert(std::is_nothrow_constructible_v<dyn_array<unsigned char>, memory_resource_type*>, 
            "Container constructor must be noexcept!");
        //static_assert(std::is_nothrow_constructible_v<dyn_array<message_locator>, memory_resource_type*>,
        //    "Container constructor must be noexcept!");
        //static_assert(std::is_trivially_copyable_v<message_locator>,
        //    "message_locator must be trivially copyable for memcpy/memmove/qsort usage!");
        //static_assert(std::is_standard_layout_v<internal_msg_header> && std::is_standard_layout_v<stamp_type>,
        //    "types must be standard layout for offsetof()!");
    };
}
