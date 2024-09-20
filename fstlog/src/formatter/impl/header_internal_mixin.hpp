//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstring>
#pragma intrinsic(memcpy)

#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/internal_msg_header.hpp>

namespace fstlog {
    //useable only with the internal decoder (endianness, alignment issues)
    template<typename L>
    class header_internal_mixin : public L {
    public:
        using allocator_type = typename L::allocator_type;
        
		header_internal_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(header_internal_mixin(allocator_type{})))
			: header_internal_mixin(allocator_type{}) {}
		explicit header_internal_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator)))
            : L(allocator) {}

		header_internal_mixin(const header_internal_mixin& other) noexcept(
			noexcept(header_internal_mixin::get_allocator())
			&& noexcept(header_internal_mixin(header_internal_mixin{}, allocator_type{})))
            : header_internal_mixin(other, other.get_allocator()) {}
        header_internal_mixin(const header_internal_mixin& other, allocator_type const& allocator) noexcept(
			noexcept(L(header_internal_mixin{}, allocator_type{})))
			: L(other, allocator) {}

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
            memcpy(&header, header_data_, internal_msg_header::unpadded_data_size);
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
            memcpy(&out,
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
            memcpy(&out,
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
            memcpy(&out,
                header_data_ + offsetof(internal_msg_header, argnum),
                sizeof(msg_counter));
            return out;
        }
                
    private:
        const unsigned char* header_data_{ nullptr };
    };
}
