//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/log_element_type.hpp>

#include <type_traits>

#include <fstlog/detail/has_input_iterator.hpp>
#include <fstlog/detail/is_char_type.hpp>
#include <fstlog/detail/is_string_like.hpp>
#include <fstlog/detail/is_tuple.hpp>
#include <fstlog/detail/rm_cvref_t.hpp>
#include <fstlog/detail/str_hash_fnv.hpp>
#include <fstlog/detail/small_string.hpp>
#include <fstlog/detail/is_pow2.hpp>

namespace fstlog {
	template <typename T, typename = void>
	struct log_type {
		static constexpr log_element_type value{ 
			log_element_type::Unloggable 
		};
	};

	template <typename T>
	struct log_type < T, std::enable_if_t<
		std::is_same_v<rm_cvref_t<T>, bool>
		>> {
		static constexpr log_element_type value{ 
			log_element_type::Bool 
		};
	};

	template <typename T>
	struct log_type < T, std::enable_if_t<
		is_char_type_v<T>
		>> {
		static constexpr log_element_type value{ 
			log_element_type::Char 
		};
	};

	template <typename T>
	struct log_type < T, std::enable_if_t<
		!std::is_same_v<rm_cvref_t<T>, bool>
		&& !is_char_type_v<T>
		&& std::is_integral_v<std::remove_reference_t<T>>
		>> {
		static constexpr log_element_type value{ 
			log_element_type::Integral 
		};
	};

	template <typename T>
	struct log_type < T, std::enable_if_t<
		std::is_floating_point_v<std::remove_reference_t<T>>
		>> {
		static constexpr log_element_type value{ 
			log_element_type::Float 
		};
	};

	template <typename T>
	struct log_type < T, std::enable_if_t<
		is_string_like_v<T>
		&& ! is_small_string<T>::value
		>> {
		static constexpr log_element_type value{ 
			log_element_type::String 
		};
	};

	template <typename T>
	struct log_type < T, std::enable_if_t<
		is_small_string<T>::value
		&& is_pow2(sizeof(T))
		&& sizeof(T) >= 16
		&& sizeof(T) <= 256
		>> {
		static constexpr log_element_type value{
			log_element_type::Smallstring
		};
	};

	template <typename T>
	struct log_type < T, std::enable_if_t<
		std::is_pointer_v<std::remove_reference_t<T>>
		>> {
		static constexpr log_element_type value{
			log_element_type::Pointer
		};
	};

	template <typename T>
	struct log_type < T, std::enable_if_t<
		std::is_same_v<rm_cvref_t<T>, str_hash_fnv>
		>> {
		static constexpr log_element_type value{ 
			log_element_type::Hash
		};
	};

	template <typename T>
	struct log_type < T, std::enable_if_t<
		(!is_string_like_v<T> && has_input_iterator_v<T>)
		|| is_tuple_v<T>
		>> {
		static constexpr log_element_type value{ 
			log_element_type::Aggregate 
		};
	};

	template <typename T>
	constexpr log_element_type log_type_v = log_type<T>::value;
}
