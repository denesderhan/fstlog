//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>

#include <fstlog/detail/internal_arg_header.hpp>
#include <fstlog/logger/detail/log_element_counter_overhead.hpp>
#include <fstlog/logger/detail/log_element_payload_size.hpp>

namespace fstlog {
	template<typename T>
	inline constexpr std::uintmax_t log_arg_size(T const& arg) noexcept {
		return std::uintmax_t{ internal_arg_header<T>::data_size }
			+ log_element_counter_overhead(arg)
			+ log_element_payload_size(arg);
	}

	template<typename T>
	inline constexpr std::uintmax_t log_arg_size() noexcept {
		return std::uintmax_t{ internal_arg_header<T>::data_size }
			+ log_element_counter_overhead<T>()
			+ log_element_payload_size<T>();
	}
}
