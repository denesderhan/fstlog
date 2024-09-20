//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <array>
#include <limits>

#include <fstlog/detail/aggregate_type.hpp>
#include <fstlog/detail/array_concat.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/log_element_type.hpp>
#include <fstlog/detail/log_type.hpp>
#include <fstlog/detail/log_type_metadata.hpp>
#include <fstlog/detail/padded_size.hpp>
#include <fstlog/detail/rm_cvref_t.hpp>
#include <fstlog/detail/types.hpp>
#include <fstlog/detail/ut_cast.hpp>

namespace fstlog {
	namespace {
		template<typename T, typename = void>
		struct type_signature_head {
			static constexpr std::array<log_element_ut, 1> value{
				{ ut_cast(log_type_v<T>) | log_type_metadata_v<T> }
			};
		};

		template <typename T>
		struct type_signature_head < T, std::enable_if_t<
			log_type_v<T> == log_element_type::Aggregate &&
			log_type_metadata_v<T> == ut_cast(aggregate_type::Tuple)
			>> {
			static_assert(std::tuple_size_v<rm_cvref_t<T>> <=
				(std::numeric_limits<log_element_ut>::max)(), 
				"Max tuple size exceeded!"
			);
			static constexpr std::array<log_element_ut, 2> value{
				ut_cast(log_element_type::Aggregate) | ut_cast(aggregate_type::Tuple),
				std::tuple_size_v<rm_cvref_t<T>>
			};
		};
	}

	template <typename T, typename = void>
	struct type_signature {
		static constexpr auto value{
			type_signature_head<T>::value
		};
	};

	template <typename T>
	constexpr auto type_signature_v = type_signature<T>::value;

	namespace {
		template<typename Tuple, std::size_t... Ints>
		inline constexpr auto tuple_type_signature(std::index_sequence<Ints...>) noexcept {
			return array_concat(
				type_signature_head<Tuple>::value, 
				type_signature_v<std::tuple_element_t<Ints, Tuple>>...);
		}
	}

	template <typename T>
	struct type_signature < T, std::enable_if_t<
		log_type_v<T> == log_element_type::Aggregate &&
		log_type_metadata_v<T> == ut_cast(aggregate_type::List)
		>> {
		static constexpr auto value = [] {
			return array_concat(
				type_signature_head<T>::value,
				type_signature<typename rm_cvref_t<T>::value_type>::value
			);
		}();
	};
	
	template <typename T>
	struct type_signature < T, std::enable_if_t<
		log_type_v<T> == log_element_type::Aggregate &&
		log_type_metadata_v<T> == ut_cast(aggregate_type::Tuple)
		>> {
		static constexpr auto value = [] {
			return tuple_type_signature<rm_cvref_t<T>>(
				std::make_index_sequence<std::tuple_size_v<rm_cvref_t<T>>>{}
			);
		}();
	};
}
