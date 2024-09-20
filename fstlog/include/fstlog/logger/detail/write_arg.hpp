//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstring>
#pragma intrinsic(memcpy)

#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/internal_arg_header.hpp>
#include <fstlog/detail/types.hpp>
#include <fstlog/logger/detail/can_log_type.hpp>
#include <fstlog/logger/detail/log_arg_size.hpp>
#include <fstlog/logger/detail/log_arg_size_compile_time.hpp>
#include <fstlog/logger/detail/write_var.hpp>
#ifdef FSTLOG_NO_NESTING
#include <fstlog/logger/detail/is_nested_type.hpp>
#endif

namespace fstlog {
	template<typename T>
	void write_arg(unsigned char*& buff_ind, T const& arg) noexcept {
		static_assert(can_log_type_v<T>, "Log argument not loggable!");
#ifdef FSTLOG_NO_NESTING
		static_assert(!is_nested_type<T>::value, "Container nesting disabled!");
#endif
		constexpr bool compt_size = log_arg_size_compile_time<T>::value;
		if constexpr (compt_size) {
			constexpr auto arg_header =	
				internal_arg_header<T>{ static_cast<fstlog::msg_counter>(log_arg_size<T>()) };
			FSTLOG_ASSERT(reinterpret_cast<std::uintptr_t>(buff_ind) % constants::internal_msg_data_alignment == 0);
			memcpy(buff_ind, &arg_header, internal_arg_header<T>::data_size);
			buff_ind += internal_arg_header<T>::data_size;
			write_var(buff_ind, arg);
		}
		else {
			auto begin_ptr = buff_ind;
			buff_ind += internal_arg_header<T>::data_size;
			write_var(buff_ind, arg);
			// log message size was calculated (log_compute_msgsize_mixin) 
			// and fits into counter, static_cast is safe
			const auto arg_header =
				internal_arg_header<T>{ static_cast<msg_counter>(buff_ind - begin_ptr) };
			FSTLOG_ASSERT(reinterpret_cast<std::uintptr_t>(begin_ptr) % constants::internal_msg_data_alignment == 0);
			memcpy(begin_ptr, &arg_header, internal_arg_header<T>::data_size);
		}
	}
}
