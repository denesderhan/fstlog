//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>
#include <limits>
#include <type_traits>

#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/internal_msg_header.hpp>
#include <fstlog/detail/level.hpp>
#include <fstlog/detail/log_metaargs.hpp>
#include <fstlog/detail/padded_size.hpp>
#include <fstlog/detail/ut_cast.hpp>
#include <fstlog/logger/detail/log_arg_size.hpp>
#include <fstlog/logger/detail/log_arg_size_compile_time.hpp>

namespace fstlog {
	template<class L>
	class log_compute_msgsize_mixin : public L {
	public:
		template<
			level level, 
			template<class T> class policy, 
			log_call_flag flags, 
			typename... Args>
		void log(Args const&... args) noexcept(
			noexcept(this->compute_msg_size(args...))
			&& noexcept(L{}.template log<level, policy, flags>(args...)))
		{
			compute_msg_size(args...);
			L::template log<level, policy, flags>(args...);
		}
		template<
			template<class T> class policy,
			log_call_flag flags,
			typename... Args>
		void log(level level, Args const&... args) noexcept(
			noexcept(this->compute_msg_size(args...))
			&& noexcept(L{}.template log<policy, flags>(level, args...)))
		{
			compute_msg_size(args...);
			L::template log<policy, flags>(level, args...);
		}
	
		template<typename... Args>
		constexpr void compute_msg_size(Args const&... args) noexcept(
			noexcept(this->buffer().max_message_size())
			&& noexcept(this->set_msg_size(0))
			&& noexcept(this->set_arg_num(0)))
		{
			constexpr bool compt_size =
				(bool{ true } && ... && log_arg_size_compile_time<Args>::value);
			if constexpr (compt_size) {
				constexpr std::uintmax_t compt_msg_size =
					(internal_msg_header::padded_data_size + ... + log_arg_size<Args>());
				if (padded_size<constants::internal_msg_alignment, std::uintmax_t>(compt_msg_size) <= L::buffer().max_message_size()) {
					L::set_msg_size(static_cast<typename L::msg_size_type>(compt_msg_size));
					L::set_arg_num(static_cast<typename L::arg_num_type>(sizeof...(Args)));
					return;
				}
			}
			
			auto msg_size = static_cast<typename L::msg_size_type>(internal_msg_header::padded_data_size);
			typename L::arg_num_type arg_num{ 0 };
			(add_arg_size(msg_size, arg_num, args) && ...);
			L::set_msg_size(msg_size);
			L::set_arg_num(arg_num);
		}

		template<typename T>
		bool add_arg_size(
			typename L::msg_size_type& msg_size, 
			typename L::arg_num_type& arg_num, 
			const T& arg) noexcept(
				noexcept(this->buffer().max_message_size()))
		{
			const std::uintmax_t msg_s{ msg_size };
			const std::uintmax_t new_size = msg_s + log_arg_size(arg);
			if (new_size > msg_s //checking for overflow
				&& padded_size<constants::internal_msg_alignment, std::uintmax_t>(new_size) <= L::buffer().max_message_size())
			{
				msg_size = static_cast<typename L::msg_size_type>(new_size);
				arg_num++;
				return true;
			}
			else {
				return false;
			}
		}
	};
}
