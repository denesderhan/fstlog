//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <type_traits>

#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <detail/unaligned_span.hpp>

namespace fstlog {
    template<typename L>
    class output_span_mixin : public L
    {
    public:
        using memory_resource_type = typename L::memory_resource_type;

        explicit output_span_mixin(memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<L, memory_resource_type*>)
            : L(resource) {}

        output_span_mixin(const output_span_mixin& other) noexcept(
            noexcept(other.get_memory_resource())
            && std::is_nothrow_constructible_v<
                output_span_mixin,
                const output_span_mixin&,
                memory_resource_type*>)
            : output_span_mixin(other, other.get_memory_resource()) {}
        output_span_mixin(const output_span_mixin& other, memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<
                L,
                const L&,
                memory_resource_type*>)
            : L(static_cast<const L&>(other), resource) {}

        output_span_mixin(output_span_mixin&& other) = delete;
        output_span_mixin& operator=(const output_span_mixin& rhs) = delete;
        output_span_mixin& operator=(output_span_mixin&& rhs) = delete;
        
        ~output_span_mixin() = default;
       
        void output_span_init(byte_span out) noexcept {
            FSTLOG_ASSERT(out.data_bytes() != nullptr);
            output_begin_ = out.data_bytes();
            output_ptr_ = output_begin_;
            output_end_ = output_begin_ + out.size_bytes();
            FSTLOG_ASSERT(output_begin_ <= output_end_);
        }

        bool output_has_space() const noexcept {
            return output_ptr_ < output_end_;
        }
        bool output_has_space(std::size_t bytes) const noexcept {
            FSTLOG_ASSERT(output_end_ != nullptr && output_ptr_ != nullptr
                && output_end_ >= output_ptr_);
            return bytes <= static_cast<std::size_t>(output_end_ - output_ptr_);
        }

        void set_output_ptr_unchecked(unsigned char* output_ptr) noexcept {
            FSTLOG_ASSERT(
                output_ptr != nullptr
                && output_begin_ != nullptr
                && output_end_ != nullptr
                && "Can not compare nullptr to object pointer (not UB but unspecifed)");
            FSTLOG_ASSERT(output_ptr >= output_begin_ && output_ptr <= output_end_);
            output_ptr_ = output_ptr;
        }

        void advance_output(std::size_t bytes) noexcept {
            FSTLOG_ASSERT(output_end_ != nullptr && output_ptr_ != nullptr
                && output_end_ >= output_ptr_);
            if (bytes <= static_cast<std::size_t>(output_end_ - output_ptr_)) {
                output_ptr_ += bytes;
            }
            else {
                this->set_error(__FILE__, __LINE__, error_code::buff_full);
            }
        }

        void advance_output_unchecked(std::size_t bytes) noexcept {
            FSTLOG_ASSERT(output_end_ != nullptr && output_ptr_ != nullptr
                && output_end_ >= output_ptr_);
            FSTLOG_ASSERT(bytes <= static_cast<std::uintptr_t>(output_end_ - output_ptr_));
            output_ptr_ += bytes;
        }

        unsigned char* output_ptr() const noexcept {
            return output_ptr_;
        }

        unsigned char* output_begin() const noexcept {
            return output_begin_;
        }
        unsigned char* output_end() const noexcept {
            return output_end_;
        }
    private:
        unsigned char* output_begin_{ nullptr };
        unsigned char* output_ptr_{ nullptr };
        unsigned char* output_end_{ nullptr };
    };
}
