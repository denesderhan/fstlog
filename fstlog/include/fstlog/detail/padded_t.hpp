//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <type_traits>

#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/padded_size.hpp>

namespace fstlog {
	
	template<typename T>
	struct padded_t_padding {
		static constexpr std::size_t value = 
			padded_size<constants::internal_msg_data_alignment>(sizeof(T))
			- sizeof(T);
	};
	
	template<typename T>
	struct padded_t_alignment {
		static constexpr std::size_t value = alignof(T) > constants::internal_msg_data_alignment ?
			alignof(T)
			: constants::internal_msg_data_alignment;
	};

	template <typename T, std::size_t N>
	struct alignas(padded_t_alignment<T>::value) padded_t_paddnonzero
	{
		typedef T value_type;
		T value;
		unsigned char padding[N]{0};
		static constexpr std::size_t padded_data_size =	sizeof(T) + N;
	};

	template<typename T>
	struct alignas(padded_t_alignment<T>::value) padded_t_paddzero
	{
		typedef T value_type;
		T value;
		static constexpr std::size_t padded_data_size = sizeof(T);
	};

	template<typename T>
	using padded_t = std::conditional_t<
		padded_t_padding<T>::value == 0,
		padded_t_paddzero<T>,
		padded_t_paddnonzero<T, padded_t_padding<T>::value>>;
}
