//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstring>

#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/internal_msg_header.hpp>

namespace fstlog {
    //useable only with the internal decoder (endianness, alignment issues)
    template<typename L>
    class header_internal_mixin : public L {
    public:
        using memory_resource_type = typename L::memory_resource_type;
        
        explicit header_internal_mixin(memory_resource_type* resource) noexcept(
            noexcept(L(resource)))
            : L(resource) {}

        header_internal_mixin(const header_internal_mixin& other) noexcept(
            noexcept(header_internal_mixin::get_memory_resource())
            && noexcept(header_internal_mixin(header_internal_mixin{nullptr}, nullptr)))
            : header_internal_mixin(other, other.get_memory_resource()) {}
        header_internal_mixin(const header_internal_mixin& other, memory_resource_type* resource) noexcept(
            noexcept(L(header_internal_mixin{nullptr}, nullptr)))
            : L(other, resource) {}

        header_internal_mixin(header_internal_mixin&& other) = delete;
        header_internal_mixin& operator=(const header_internal_mixin& rhs) = delete;
        header_internal_mixin& operator=(header_internal_mixin&& rhs) = delete;

        ~header_internal_mixin() = default;

        void set_header(const unsigned char* header_ptr) noexcept {
            FSTLOG_ASSERT(header_ptr != nullptr);
            header_data_ = header_ptr;
        }

        void get_header(internal_msg_header& header) const noexcept {
            FSTLOG_ASSERT(header_data_ != nullptr);
            std::memcpy(&header, header_data_, internal_msg_header::unpadded_data_size);
        }

        log_msg_type message_type() const noexcept {
            FSTLOG_ASSERT(header_data_ != nullptr);
            static_assert(sizeof(internal_msg_header::msg_type) == 1);
            return log_msg_type{
                *(header_data_ + offsetof(internal_msg_header, msg_type)) };
        }

        msg_counter message_size() const noexcept {
            FSTLOG_ASSERT(header_data_ != nullptr);
            msg_counter out;
            std::memcpy(&out,
                header_data_ + offsetof(internal_msg_header, msg_size),
                sizeof(msg_counter));
            return out;
        }
        level severity() const noexcept {
            FSTLOG_ASSERT(header_data_ != nullptr);
            static_assert(sizeof(internal_msg_header::severity) == 1);
            return level{
                *(header_data_ + offsetof(internal_msg_header, severity)) };
        }
        log_call_flag header_flags() const noexcept {
            FSTLOG_ASSERT(header_data_ != nullptr);
            static_assert(sizeof(internal_msg_header::flags) == 1);
            return log_call_flag{
                *(header_data_ + offsetof(internal_msg_header, flags)) };
        }
        channel_type channel() const noexcept {
            FSTLOG_ASSERT(header_data_ != nullptr);
            static_assert(sizeof(internal_msg_header::channel) == 1);
            return channel_type{
                *(header_data_ + offsetof(internal_msg_header, channel)) };
        }
        stamp_type timestamp() const noexcept {
            FSTLOG_ASSERT(header_data_ != nullptr);
            stamp_type out;
            std::memcpy(&out,
                header_data_ + offsetof(internal_msg_header, timestamp),
                sizeof(stamp_type));
            return out;
        }
        log_policy policy() const noexcept {
            FSTLOG_ASSERT(header_data_ != nullptr);
            static_assert(sizeof(internal_msg_header::policy) == 1);
            return log_policy{
                *(header_data_ + offsetof(internal_msg_header, policy)) };
        }
        msg_counter arg_num() const noexcept {
            FSTLOG_ASSERT(header_data_ != nullptr);
            msg_counter out;
            std::memcpy(&out,
                header_data_ + offsetof(internal_msg_header, argnum),
                sizeof(msg_counter));
            return out;
        }
                
    private:
        const unsigned char* header_data_{ nullptr };
    };
}
