//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <type_traits>
#include <utility>

#include <fstlog/detail/log_type.hpp>
#include <fstlog/detail/log_type_metadata.hpp>
#include <fstlog/detail/rm_cvref_t.hpp>

namespace fstlog {
	template <typename T, typename = void>
	struct can_log_type {
		static constexpr bool value{ false };
	};

	template <typename T>
	constexpr bool can_log_type_v = can_log_type<T>::value;

	template <typename T>
	struct can_log_type < T, std::enable_if_t<
		log_type_v<T> != log_element_type::Unloggable &&
		log_type_v<T> != log_element_type::Aggregate
		>> {
		static constexpr bool value{ true };
	};
	
	template <typename T>
	struct can_log_type < T, std::enable_if_t<
		log_type_v<T> == log_element_type::Aggregate &&
		log_type_metadata_v<T> == ut_cast(aggregate_type::List)
		>> {
		static constexpr bool value{ [] {
			return can_log_type_v<typename rm_cvref_t<T>::value_type>;
		}()
		};
	};

	template<typename Tuple, std::size_t... Ints>
	inline constexpr bool can_log_tuple(std::index_sequence<Ints...>) noexcept {
		return (true && ... && can_log_type_v<std::tuple_element_t<Ints, rm_cvref_t<Tuple>>>);
	}

	template <typename T>
	struct can_log_type < T, std::enable_if_t<
		log_type_v<T> == log_element_type::Aggregate &&
		log_type_metadata_v<T> == ut_cast(aggregate_type::Tuple)
		>> {
		static constexpr bool value{ [] {
			return can_log_tuple<T>(
				std::make_index_sequence<std::tuple_size_v<rm_cvref_t<T>>>{});
		}()
		};
	};
}
