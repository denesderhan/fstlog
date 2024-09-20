//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cassert>
#include <cstring>
#include <limits>
#pragma intrinsic(memcpy)

#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/internal_msg_header.hpp>
#include <fstlog/detail/level.hpp>
#include <fstlog/detail/log_metaargs.hpp>
#include <fstlog/detail/padded_size.hpp>
#include <fstlog/detail/types.hpp>
#include <fstlog/detail/ut_cast.hpp>
#include <fstlog/logger/detail/write_arg.hpp>

namespace fstlog {
		
	template<class L>
	class logger_writer_mixin : public L {
	public:
		bool message_fits() noexcept(
			noexcept(this->buffer())
			&& noexcept(this->buffer().writeable_size())
			&& noexcept(this->msg_size()))
		{
			return L::buffer().writeable_size() >=
				padded_size<constants::internal_msg_alignment>(L::msg_size());
		}

		void update_buffer_state() noexcept(
			noexcept(this->buffer())
			&& noexcept(this->msg_size())
			&& noexcept(this->buffer().update_producer_state(this->msg_size())))
		{
			L::buffer().update_producer_state(
				padded_size<constants::internal_msg_alignment>(L::msg_size()));
		}

		void wait_for_buffer_flush() noexcept(
			noexcept(this->buffer())
			&& noexcept(this->get_core())
			&& noexcept(this->buffer().wait_for_buffer_flush(this->get_core())))
		{
			L::buffer().wait_for_buffer_flush(L::get_core());
		}

		std::uint32_t write_pos() noexcept(
			noexcept(this->buffer())
			&& noexcept(this->buffer().write_pos()))
		{
			return L::buffer().write_pos();
		}

		void request_flush_if_needed(std::uint32_t prev_write_pos) noexcept(
			noexcept(this->buffer())
			&& noexcept(this->get_core())
			&& noexcept(this->buffer().request_flush_if_needed(prev_write_pos, this->get_core())))
		{
			L::buffer().request_flush_if_needed(prev_write_pos, L::get_core());
		}

		template<
			log_policy pol,
			log_call_flag flags,
			typename... Args>
		void write_message(level level, Args const&... args) noexcept(
			noexcept(this->buffer())
			&& noexcept(this->buffer().write_ptr())
			&& noexcept(this->msg_size())
			&& noexcept(this->channel())
			&& noexcept(this->timestamp())
			&& noexcept(this->arg_num())
			&& noexcept(this->buffer().advance_write_pos(this->msg_size())))
		{
			assert(ut_cast(level) > ut_cast(level::None) &&
				ut_cast(level) < ut_cast(level::All) && "fstlog: Invalid log level!");
			auto& buffer = L::buffer();
			unsigned char* buff_ptr = buffer.write_ptr();
#ifdef FSTLOG_DEBUG
			[[maybe_unused]] const auto buff_beg = buff_ptr;
#endif
			std::uint32_t msg_size = L::msg_size();
			constexpr std::size_t orig_argnum{ sizeof...(Args) };
			const auto padded_msg_size = padded_size<constants::internal_msg_alignment>(msg_size);
			const internal_msg_header header{
				log_msg_type::Internal,
				level,
				L::channel(),
				flags,
				static_cast<msg_counter>(msg_size),
				//writing original number of args not the cropped!, can be checked if args where cropped
				static_cast<msg_counter>(orig_argnum),
				L::timestamp(),
				pol,
				{1,	0, 0}
			};
			static_assert(orig_argnum <=
				(std::numeric_limits<decltype(header.argnum)>::max)(), "Too many arguments!");
			memcpy(buff_ptr, &header, internal_msg_header::unpadded_data_size);
			if constexpr (internal_msg_header::unpadded_data_size != internal_msg_header::padded_data_size ) {
				memset(
					buff_ptr + internal_msg_header::unpadded_data_size,
					0,
					internal_msg_header::padded_data_size - internal_msg_header::unpadded_data_size);
			}
			buff_ptr += internal_msg_header::padded_data_size;
			std::uint32_t arg_num = L::arg_num();
			if (arg_num == orig_argnum) {
				(write_arg(buff_ptr, args), ...);
			}
			else {
				(log_arg_cropped(arg_num, buff_ptr, args), ...);
			}
			FSTLOG_ASSERT(buff_ptr > buff_beg && buff_ptr - buff_beg == msg_size);
			//nobody reads the padding at the end of message (sink_msgblock does not include it in the message) 
			//if (padded_msg_size != msg_size)
			//	memset(buff_ptr, 0, padded_msg_size - msg_size);
			buffer.advance_write_pos(padded_msg_size);
		}

		template<
			level level,
			log_policy pol,
			log_call_flag flags,
			typename... Args>
		void write_message(Args const&... args) noexcept(
			noexcept(write_message<pol, flags>(level, args...)))
		{
			write_message<pol, flags>(level, args...);
		}

		template<typename T>
		static void log_arg_cropped(
			std::uint32_t& arg_num, 
			unsigned char*& buff_ptr, 
			T const& arg) noexcept 
		{
			if (arg_num != 0) {
				write_arg(buff_ptr, arg);
				arg_num--;
			}
		}
	};
}
