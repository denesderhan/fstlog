//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <type_traits>

#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/types.hpp>
#include <detail/unaligned_span.hpp>

namespace fstlog {
    template<typename L>
    class input_span_mixin : public L {
    public:
        using memory_resource_type = typename L::memory_resource_type;

        explicit input_span_mixin(memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<L, memory_resource_type*>)
            : L(resource) {}

        input_span_mixin(const input_span_mixin& other) noexcept(
            noexcept(other.get_memory_resource())
            && std::is_nothrow_constructible_v<
                input_span_mixin,
                const input_span_mixin&,
                memory_resource_type*>)
            : input_span_mixin(other, other.get_memory_resource()) {}

        input_span_mixin(const input_span_mixin& other, memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<
                L,
                const input_span_mixin&,
                memory_resource_type*>)
            : L(other, resource) {}

        input_span_mixin(input_span_mixin&& other) = delete;
        input_span_mixin& operator=(const input_span_mixin& rhs) = delete;
        input_span_mixin& operator=(input_span_mixin&& rhs) = delete;

        ~input_span_mixin() = default;

        void input_span_init(byte_span_const msg) noexcept {
            FSTLOG_ASSERT(msg.data_bytes() != nullptr);
            input_msg_begin_ = msg.data_bytes();
            input_msg_ptr_ = input_msg_begin_;
            input_msg_end_ = input_msg_begin_ + msg.size_bytes();
            FSTLOG_ASSERT(input_msg_begin_ <= input_msg_end_);
        }
        
        void set_input_ptr_unchecked(unsigned char const* input_ptr) noexcept {
            FSTLOG_ASSERT(
                input_ptr != nullptr
                && input_msg_begin_ != nullptr
                && input_msg_end_ != nullptr
                && "Can not compare nullptr to object pointer (not UB but unspecifed)");
            FSTLOG_ASSERT(input_ptr >= input_msg_begin_ && input_ptr <= input_msg_end_);
            input_msg_ptr_ = input_ptr;
        }

        void advance_input(std::size_t bytes) noexcept {
            FSTLOG_ASSERT(input_msg_end_ != nullptr && input_msg_ptr_ != nullptr
                && input_msg_end_ >= input_msg_ptr_);
            if (bytes <= static_cast<std::size_t>(input_msg_end_ -  input_msg_ptr_)) {
                input_msg_ptr_ += bytes;
            }
            else {
                this->set_error(__FILE__, __LINE__, error_code::input_bad);
            }
        }

        void advance_input_unchecked(std::size_t bytes) noexcept {
            FSTLOG_ASSERT(input_msg_end_ != nullptr && input_msg_ptr_ != nullptr
                && input_msg_end_ >= input_msg_ptr_);
            FSTLOG_ASSERT(bytes <= static_cast<std::size_t>(input_msg_end_ - input_msg_ptr_));
            input_msg_ptr_ += bytes;
        }

        unsigned char const* input_ptr() const noexcept {
            return input_msg_ptr_;
        }

        bool end_of_input_data() const noexcept {
            return (input_msg_ptr_ >= input_msg_end_);
        }

        unsigned char const* input_begin() const noexcept {
            return input_msg_begin_;
        }
        unsigned char const* input_end() const noexcept {
            return input_msg_end_;
        }

    private:
        unsigned char const* input_msg_begin_{ nullptr };
        unsigned char const* input_msg_ptr_{ nullptr };
        unsigned char const* input_msg_end_{ nullptr };
    };
}
