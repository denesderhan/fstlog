//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstddef>
#include <type_traits>

#include <fstlog/detail/aggregate_type.hpp>
#include <fstlog/detail/is_stdarray.hpp>
#include <fstlog/detail/log_element_type.hpp>
#include <fstlog/detail/log_type.hpp>
#include <fstlog/detail/log_type_metadata.hpp>
#include <fstlog/detail/rm_cvref_t.hpp>

namespace fstlog {
	template <typename T, typename = void>
	struct log_arg_size_compile_time : std::false_type {};
	
	template <typename T>
	struct log_arg_size_compile_time < T, std::enable_if_t<
		log_type_v<T> != log_element_type::String
		&& log_type_v<T> != log_element_type::Aggregate
		&& log_type_v<T> != log_element_type::Unloggable
		>> : std::true_type {};
	
	//string literal / char[] (log_element_type::String)
	template <typename T, std::size_t N>
	struct log_arg_size_compile_time <T[N], std::enable_if_t<
		is_char_type_v<T>
		>> : std::true_type{};

	template <typename T, std::size_t N>
	struct log_arg_size_compile_time <T(&)[N], std::enable_if_t<
		is_char_type_v<T>
		>> : std::true_type{};

	namespace {
		template<typename Tuple, std::size_t... Ints>
		inline constexpr bool tuple_size_compile_time(std::index_sequence<Ints...>) noexcept {
			return (bool{ true } && ... && 
				log_arg_size_compile_time<std::tuple_element_t<Ints, Tuple>>::value);
		}
	}

	//tuple
	template <typename T>
	struct log_arg_size_compile_time < T, std::enable_if_t <
		log_type_v<T> == log_element_type::Aggregate
		&& log_type_metadata_v<T> == ut_cast(aggregate_type::Tuple)>> {
		static constexpr bool value =
			tuple_size_compile_time<rm_cvref_t<T>>(std::make_index_sequence<std::tuple_size_v<rm_cvref_t<T>>>{});
	};

	//list
	template <typename T>
	struct log_arg_size_compile_time < T, std::enable_if_t <
		log_type_v<T> == log_element_type::Aggregate
		&& log_type_metadata_v<T> == ut_cast(aggregate_type::List)>> {
		static constexpr bool value = [] {
			if constexpr (is_stdarray<rm_cvref_t<T>>::value) {
				return log_arg_size_compile_time<typename rm_cvref_t<T>::value_type>::value;
			}
			else return false;
		}();
	};
}
