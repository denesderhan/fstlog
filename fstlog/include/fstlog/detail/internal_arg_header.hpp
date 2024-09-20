//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

#include <fstlog/detail/types.hpp>
#include <fstlog/detail/type_signature.hpp>
#include <fstlog/detail/padded_size.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/array_copy.hpp>

namespace fstlog {
	template <typename T>
	struct alignas(constants::internal_msg_data_alignment) internal_arg_header {
		static constexpr std::size_t data_size = 
			padded_size<constants::internal_msg_data_alignment>(
				sizeof(msg_counter)
				+ 1 
				+ type_signature<T>::value.size());
	private:
		static constexpr std::uintmax_t array_length = data_size - sizeof(msg_counter) - 1;
		static constexpr std::array<unsigned char, array_length> padded_sign = [] {
			std::array<unsigned char, array_length> out{ 0 };
			array_copy(out, type_signature<T>::value, 0);
			return out;
		}();
	public:	
		msg_counter arg_size{0};
		const unsigned char signature_length = static_cast<unsigned char>(type_signature<T>::value.size());
		const std::array<log_element_ut, array_length> signature = padded_sign;
		static_assert(type_signature<T>::value.size() <= (std::numeric_limits<unsigned char>::max)(),
			"Error, log argument signature too long!");
#ifdef FSTLOG_DEBUG
		static_assert(std::is_same_v<log_element_ut, unsigned char>);
		static_assert(constants::internal_msg_data_alignment >= alignof(msg_counter)
			&& constants::internal_msg_data_alignment % alignof(msg_counter) == 0);
#endif
	};
#ifdef FSTLOG_DEBUG
	static_assert(sizeof(internal_arg_header<int>) == internal_arg_header<int>::data_size);
#endif
}
