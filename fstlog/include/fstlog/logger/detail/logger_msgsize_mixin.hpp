//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/types.hpp>

namespace fstlog {
	template<class L>
	class logger_msgsize_mixin : public L {
	public:
		typedef std::uint32_t msg_size_type;
		typedef std::uint32_t arg_num_type;
		
		msg_size_type msg_size() const noexcept {
			return msg_size_;
		}

		arg_num_type arg_num() const noexcept {
			return arg_num_;
		}

		void set_msg_size(msg_size_type size) noexcept {
			msg_size_ = size;
		}

		void set_arg_num(arg_num_type num) noexcept {
			arg_num_ = num;
		}

		void inc_arg_num() noexcept {
			arg_num_++;
		}

		msg_size_type msg_size_{ 0 };
		arg_num_type arg_num_{ 0 };
	};
}

