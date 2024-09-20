//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/convert_to_basic_string_view.hpp>
#include <fstlog/detail/log_element_type.hpp>
#include <fstlog/detail/log_type.hpp>
#include <fstlog/detail/log_type_metadata.hpp>
#include <fstlog/detail/padded_size.hpp>
#include <fstlog/detail/rm_cvref_t.hpp>
#include <fstlog/logger/detail/log_arg_size_compile_time.hpp>

namespace fstlog {
	namespace {
		template<class Tuple, std::size_t... Ints>
		constexpr std::uintmax_t tuple_payload_size(Tuple const& tuple, std::index_sequence<Ints...>) noexcept;

		template<class Tuple, std::size_t... Ints>
		constexpr std::uintmax_t tuple_payload_size(std::index_sequence<Ints...>) noexcept;
	}

	template<typename T, typename std::enable_if_t<
		log_type_v<T> != log_element_type::Aggregate
	>* = nullptr>
	constexpr std::uintmax_t log_element_payload_size() noexcept {
		//string literal
		if constexpr (log_type_v<T> == log_element_type::String) {
			//assert string literal
			static_assert(log_arg_size_compile_time<T>::value);
			constexpr std::uintmax_t char_bytes{ sizeof(std::remove_pointer_t<std::decay_t<T>>) };
			constexpr std::uintmax_t byte_size{ sizeof(T) - char_bytes };
			return padded_size<constants::internal_msg_data_alignment>(byte_size);
		}
		//string ptr, small_string, Char, Bool, Float, Integral, Hash, Pointer
		else {
			return padded_size<constants::internal_msg_data_alignment>(sizeof(T));
		}
	}

	template<typename T, typename std::enable_if_t<
		log_type_v<T> == log_element_type::Aggregate
	>* = nullptr>
	constexpr std::uintmax_t log_element_payload_size() noexcept {
		if constexpr (log_type_metadata_v<T> == ut_cast(aggregate_type::List)) {
			//assert std::array
			static_assert(is_stdarray<rm_cvref_t<T>>::value);
			constexpr std::uintmax_t list_size{ std::tuple_size_v<rm_cvref_t<T>> };
			return list_size * log_element_payload_size< typename rm_cvref_t<T>::value_type >();
		}
		else if constexpr (log_type_metadata_v<T> == ut_cast(aggregate_type::Tuple)) {
			return tuple_payload_size<rm_cvref_t<T>>(std::make_index_sequence<std::tuple_size_v<rm_cvref_t<T>>>{});
		}
		else {
			static_assert(!sizeof(T), "fstlog: Can not compute log element payload size!");
			return 0;
		}
	}

	template<typename T, typename std::enable_if_t<
		log_type_v<T> != log_element_type::Aggregate
		>* = nullptr>
	constexpr std::uintmax_t log_element_payload_size([[maybe_unused]] T const& element) noexcept {
		//string
		if constexpr (log_type_v<T> == log_element_type::String) {
			const auto str_v = convert_to_basic_string_view(element);
			typedef typename decltype(str_v)::value_type value_type ;
			return padded_size<constants::internal_msg_data_alignment>(
				static_cast<std::uintmax_t>(str_v.size()) * sizeof(value_type));
		}
		//small_string, Char, Bool, Float, Integral, Hash, Pointer
		else {
			return padded_size<constants::internal_msg_data_alignment>(sizeof(T));
		}
	}
	
	template<typename T, typename std::enable_if_t<
		log_type_v<T> == log_element_type::Aggregate
		>* = nullptr>
	constexpr std::uintmax_t log_element_payload_size(T const& element) noexcept {
		if constexpr (log_type_metadata_v<T> == ut_cast(aggregate_type::List)) {
			std::uintmax_t payload_size = 0;
			if constexpr (log_type_v<typename T::value_type> != log_element_type::Aggregate &&
				log_type_v<typename T::value_type> != log_element_type::String)
			{
				payload_size = log_element_payload_size<typename T::value_type>()
					* element.size();
			}
			else {
				for (const auto& e : element) {
					payload_size += log_element_payload_size(e);
				}
			}
			return payload_size;
		}
		else if constexpr (log_type_metadata_v<T> == ut_cast(aggregate_type::Tuple)) {
			return tuple_payload_size(element, std::make_index_sequence<std::tuple_size_v<rm_cvref_t<T>>>{});
		}
		else {
			static_assert(!sizeof(T), "fstlog: Can not compute log element payload size!");
			return 0;
		}
	}

	namespace {
		template<class Tuple, std::size_t... Ints>
		constexpr std::uintmax_t tuple_payload_size(Tuple const& tuple, std::index_sequence<Ints...>) noexcept {
			return (std::uintmax_t{ 0 } + ... + log_element_payload_size(std::get<Ints>(tuple)));
		}

		template<class Tuple, std::size_t... Ints>
		constexpr std::uintmax_t tuple_payload_size(std::index_sequence<Ints...>) noexcept {
			return (std::uintmax_t{ 0 } + ... + log_element_payload_size<typename std::tuple_element<Ints, Tuple>::type>());
		}
	}
}
