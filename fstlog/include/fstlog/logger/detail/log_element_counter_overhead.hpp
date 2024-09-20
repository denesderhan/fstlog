//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <limits>
#include <type_traits>

#ifdef FSTLOG_DEBUG
#include <fstlog/detail/is_stdarray.hpp>
#endif
#include <fstlog/detail/log_element_type.hpp>
#include <fstlog/detail/log_type.hpp>
#include <fstlog/detail/log_type_metadata.hpp>
#include <fstlog/detail/padded_size.hpp>
#include <fstlog/detail/type_signature.hpp>

namespace fstlog {
	inline constexpr std::size_t counter_size_from_type(log_element_ut log_type) noexcept {
		auto type = log_type & log_element_type_bitmask;
		if (type == ut_cast(log_element_type::String)) {
			return padded_size<constants::internal_msg_data_alignment, std::size_t>(sizeof(msg_counter));
		}
		else if (type == ut_cast(log_element_type::Aggregate) &&
			(log_type & log_type_metadata_bitmask) == ut_cast(aggregate_type::List)) {
			return padded_size<constants::internal_msg_data_alignment, std::size_t>(sizeof(msg_counter));
		}
		else {
			return 0;
		}
	}

	template <typename T>
	inline constexpr std::size_t counter_size_from_type() noexcept {
		return counter_size_from_type(type_signature_head<T>::value[0]);
	}

	template<typename T, typename std::enable_if_t<
		log_type_v<T> != log_element_type::Aggregate
		>* = nullptr>
	inline constexpr std::size_t log_element_counter_overhead([[maybe_unused]] const T& element) noexcept {
		return counter_size_from_type<T>();
	}

	template<typename Tuple, std::size_t... Ints>
	inline constexpr std::size_t tuple_counter_overhead(const Tuple& tuple, std::index_sequence<Ints...>) noexcept;

	template<typename T, typename std::enable_if_t<
		log_type_v<T> == log_element_type::Aggregate
		>* = nullptr>
	inline constexpr std::size_t log_element_counter_overhead([[maybe_unused]] const T& element) noexcept {
		std::size_t overhead{ counter_size_from_type<T>() };
		if constexpr (log_type_metadata_v<T> == ut_cast(aggregate_type::List)) {
			if constexpr (log_type_v<typename T::value_type> != log_element_type::Aggregate) {
				constexpr std::size_t elem_elem_counter_size{
					counter_size_from_type<typename rm_cvref_t<T>::value_type>()};
				if constexpr (elem_elem_counter_size != 0) {
					overhead += elem_elem_counter_size * element.size();
				}
			}
			else {
				for (const auto& e : element) {
					overhead += log_element_counter_overhead(e);
				}
			}
		}
		else if constexpr (log_type_metadata_v<T> == ut_cast(aggregate_type::Tuple)) {
			overhead += tuple_counter_overhead(element, std::make_index_sequence<std::tuple_size_v<rm_cvref_t<T>>>{});
		}
		else {
			static_assert(!sizeof(T), "fstlog: Can not compute element counter overhead!");
		}
		return overhead;
	}

	template<typename T, typename std::enable_if_t<
		log_type_v<T> != log_element_type::Aggregate
	>* = nullptr>
	inline constexpr std::size_t log_element_counter_overhead() noexcept {
		return counter_size_from_type<rm_cvref_t<T>>();
	}

	template<typename Tuple, std::size_t... Ints>
	inline constexpr std::size_t tuple_counter_overhead(std::index_sequence<Ints...>) noexcept;

	template<typename T, typename std::enable_if_t<
		log_type_v<T> == log_element_type::Aggregate
		>* = nullptr>
	inline constexpr std::size_t log_element_counter_overhead() noexcept {
		std::size_t overhead = counter_size_from_type<rm_cvref_t<T>>();
		if constexpr (log_type_metadata_v<T> == ut_cast(aggregate_type::List)) {
			FSTLOG_ASSERT(is_stdarray<rm_cvref_t<T>>::value && "This can only be an std::array!");
			overhead += counter_size_from_type<typename rm_cvref_t<T>::value_type>() 
				* std::tuple_size_v<rm_cvref_t<T>>;
		}
		else if constexpr (log_type_metadata_v<T> == ut_cast(aggregate_type::Tuple)) {
			overhead += tuple_counter_overhead<T>(std::make_index_sequence<std::tuple_size_v<rm_cvref_t<T>>>{});
		}
		else {
			static_assert(!sizeof(T), "fstlog: Can not compute element counter overhead!");
		}
		return overhead;
	}

	template<typename Tuple, std::size_t... Ints>
	inline constexpr std::size_t tuple_counter_overhead(const Tuple& tuple, std::index_sequence<Ints...>) noexcept {
		return (std::size_t{ 0 } + ... + log_element_counter_overhead(std::get<Ints>(tuple)));
	}

	template<typename Tuple, std::size_t... Ints>
	inline constexpr std::size_t tuple_counter_overhead(std::index_sequence<Ints...>) noexcept {
		return (std::size_t{ 0 } + ... + log_element_counter_overhead<std::tuple_element_t<Ints, rm_cvref_t<Tuple>>>());
	}
}
