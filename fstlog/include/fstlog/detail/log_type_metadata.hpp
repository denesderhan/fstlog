//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <limits>
#include <string_view>
#include <type_traits>

#include <fstlog/detail/aggregate_type.hpp>
#include <fstlog/detail/char_type.hpp>
#include <fstlog/detail/hash_type.hpp>
#include <fstlog/detail/log_type.hpp>
#include <fstlog/detail/log2_size.hpp>
#include <fstlog/detail/ut_cast.hpp>

namespace fstlog {

	template <typename T, typename = void>
	struct log_type_metadata {
		static_assert(!sizeof(T), "Error, no log_type_metadata defined!");
		static constexpr log_element_ut value{};
	};

	template <typename T>
	struct log_type_metadata < T, std::enable_if_t<
		log_type_v<T> == log_element_type::Unloggable
		>> {
		static constexpr log_element_ut value{ 0 };
	};

	template <typename T>
	struct log_type_metadata < T, std::enable_if_t<
		std::is_same_v<rm_cvref_t<T>, char>
		|| (log_type_v<T> == log_element_type::String && std::is_convertible_v<rm_cvref_t<T>, std::string_view>)
		>> {
		static constexpr log_element_ut value{ ut_cast(char_type::Char) };
	};

#ifdef __cpp_char8_t
	template <typename T>
	struct log_type_metadata < T, std::enable_if_t<
		std::is_same_v<rm_cvref_t<T>, char8_t>
		|| (log_type_v<T> == log_element_type::String && std::is_convertible_v<rm_cvref_t<T>, std::u8string_view>)
		>> {
		static constexpr log_element_ut value{ ut_cast(char_type::Char8) };
	};
#endif

	template <typename T>
	struct log_type_metadata < T, std::enable_if_t<
		std::is_same_v<rm_cvref_t<T>, char16_t>
		|| (log_type_v<T> == log_element_type::String && std::is_convertible_v<rm_cvref_t<T>, std::u16string_view>)
		>> {
		static constexpr log_element_ut value{ ut_cast(char_type::Char16) };
	};

	template <typename T>
	struct log_type_metadata < T, std::enable_if_t<
		std::is_same_v<rm_cvref_t<T>, char32_t>
		|| (log_type_v<T> == log_element_type::String && std::is_convertible_v<rm_cvref_t<T>, std::u32string_view>)
		>> {
		static constexpr log_element_ut value{ ut_cast(char_type::Char32) };
	};

	template <typename T>
	struct log_type_metadata < T, std::enable_if_t<
		log_type_v<T> == log_element_type::Aggregate
		&& has_input_iterator_v<T>
		>> {
		static constexpr log_element_ut value{ ut_cast(aggregate_type::List)};
	};

	template <typename T>
	struct log_type_metadata < T, std::enable_if_t<
		log_type_v<T> == log_element_type::Aggregate
		&& is_tuple_v<T>
		>> {
		static constexpr log_element_ut value{ ut_cast(aggregate_type::Tuple) };
	};

	template <typename T>
	struct log_type_metadata < T, std::enable_if_t<
		log_type_v<T> == log_element_type::Integral
		>> {
		static constexpr log_element_ut value{
			//sign
			(log_element_ut {std::is_signed_v<rm_cvref_t<T>>} << 3)
			//size
			| log2_size<T>::value
		};
	};

	template <typename T>
	struct log_type_metadata < T, std::enable_if_t<
		log_type_v<T> == log_element_type::Float
		>> {
		static constexpr log_element_ut value{ log2_size<T>::value };
	};

	template <typename T>
	struct log_type_metadata < T, std::enable_if_t <
		log_type_v<T> == log_element_type::Bool
		>> {
		static constexpr log_element_ut value{ 0 };
	};

	template <typename T>
	struct log_type_metadata < T, std::enable_if_t <
		std::is_same_v<rm_cvref_t<T>, str_hash_fnv>
		>> {
		static constexpr log_element_ut value{ ut_cast(hash_type::Fnv) };
	};

	template <typename T>
	struct log_type_metadata < T, std::enable_if_t<
		log_type_v<T> == log_element_type::Pointer
		>> {
		static constexpr log_element_ut value{ 0 };
	};

	template <typename T>
	struct log_type_metadata < T, std::enable_if_t<
		log_type_v<T> == log_element_type::Smallstring
		>> {
		static constexpr log_element_ut value{ sizeof(T) >> 5};
	};

	template <typename T>
	constexpr log_element_ut log_type_metadata_v = log_type_metadata<T>::value;
	inline constexpr log_element_ut log_type_metadata_bitmask = 0x0f;
}
