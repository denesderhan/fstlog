//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <chrono>
#include <cstring>
#pragma intrinsic(memcpy)

#include <detail/byte_span.hpp>
#include <detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/internal_msg_header.hpp>
#include <fstlog/detail/padded_size.hpp>
#include <fstlog/detail/types.hpp>
#include <fstlog/detail/ut_cast.hpp>

namespace fstlog {
    template<class L>
    class sink_msgblock_mixin : public L {
    public:
        using allocator_type = typename L::allocator_type;

		sink_msgblock_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(sink_msgblock_mixin(allocator_type{})))
			: sink_msgblock_mixin(allocator_type{}) {}
		explicit sink_msgblock_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
            : L(allocator) {}

        sink_msgblock_mixin(const sink_msgblock_mixin& other) = delete;
        sink_msgblock_mixin(sink_msgblock_mixin&& other) = delete;
        sink_msgblock_mixin& operator=(const sink_msgblock_mixin& rhs) = delete;
        sink_msgblock_mixin& operator=(sink_msgblock_mixin&& rhs) = delete;

        ~sink_msgblock_mixin() = default;
    
        error_code sink_msg_block(const unsigned char* begin, std::uint32_t block_size) noexcept {
			FSTLOG_ASSERT(begin != nullptr);
            std::uint32_t remaining_data = block_size;
            auto dat_ptr = begin;
            error_code errc{ error_code::none };
            bool has_sinked{ false };
            while(true) {
				//first byte of message is log_msg_type (it has to be log_msg_type::Internal)
				static_assert(offsetof(internal_msg_header, msg_type) == 0);
				static_assert(sizeof(internal_msg_header::msg_type) == 1);
				if (remaining_data == 0 || *dat_ptr != ut_cast(log_msg_type::Internal)) break;
				if (remaining_data < internal_msg_header::padded_data_size) {
					errc = error_code::input_contract_violation;
					break;
				}
				msg_counter msg_size;
                memcpy(
                    &msg_size,
                    dat_ptr + offsetof(internal_msg_header, msg_size),
                    sizeof(msg_counter));
                const auto padded_msg_size = padded_size<constants::internal_msg_alignment, std::uint32_t>(msg_size);
                if (msg_size >= internal_msg_header::padded_data_size && padded_msg_size <= remaining_data) {
					static_assert(sizeof(level) == 1 && sizeof(channel_type) == 1);
					const level level{ *(dat_ptr + offsetof(internal_msg_header, severity)) };
					const channel_type channel{ *(dat_ptr + offsetof(internal_msg_header, channel)) };
					if (L::filter_msg(level, channel)) {
                        L::sink_msg(buff_span_const{ dat_ptr, msg_size });
                        has_sinked = true;
                    }  
                    remaining_data -= padded_msg_size;
                    dat_ptr += padded_msg_size;
                }
                else {
                    errc = error_code::input_contract_violation;
                    break;
                }
            }
            if (has_sinked)
				L::set_unflushed_data();
            return errc;
        }
    };
}
