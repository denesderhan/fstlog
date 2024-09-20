//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <type_traits>
#include <cstring>
#pragma intrinsic(memcpy, memset)

#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/convert_to_basic_string_view.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/log_type.hpp>
#include <fstlog/detail/log_type_metadata.hpp>
#include <fstlog/detail/padded_size.hpp>
#include <fstlog/detail/padded_t.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
	template<typename T, typename std::enable_if_t<
			log_type_v<T> != log_element_type::Aggregate
			&& log_type_v<T> != log_element_type::String
		>* = nullptr>
	void write_var(unsigned char*& buff_ind, T const& var) noexcept {
		const padded_t<rm_cvref_t<T>> temp{ var };
		memcpy(buff_ind, &temp, padded_t<rm_cvref_t<T>>::padded_data_size);
		buff_ind += padded_t<rm_cvref_t<T>>::padded_data_size;
	}

	//string contents copied
	template<typename T, typename std::enable_if_t<
		log_type_v<T> == log_element_type::String
		>* = nullptr>
	void write_var(unsigned char*& buff_ind, T const& var) noexcept {
		const auto string_var = convert_to_basic_string_view(var);
		const auto string_size = static_cast<msg_counter>(string_var.size());
		FSTLOG_ASSERT(string_var.size() == string_size);
		write_var(buff_ind, string_size);
		const std::size_t byte_size = sizeof(typename decltype(string_var)::value_type) * string_size;
		FSTLOG_ASSERT(byte_size >= string_size);
		FSTLOG_ASSERT(reinterpret_cast<std::uintptr_t>(buff_ind) % constants::internal_msg_data_alignment == 0);
		//preventing undefined behavior when string_var.data() == nullptr
		if (byte_size != 0) {
			memcpy(buff_ind, string_var.data(), byte_size);
			const std::size_t padded_str_size{ padded_size<constants::internal_msg_data_alignment>(byte_size) };
			const std::size_t bytes_to_zero{ padded_str_size - byte_size };
			memset(buff_ind + byte_size, 0, bytes_to_zero);
			buff_ind += padded_str_size;
		}
	}

	namespace {
		template<typename Tuple, std::size_t... Ints>
		void write_tuple(unsigned char*& buff_ind, Tuple const& tuple, std::index_sequence<Ints...>) noexcept;
	}

	//aggregate types
	template<typename T, typename std::enable_if_t<
		log_type_v<T> == log_element_type::Aggregate, 
		T>* = nullptr>
	void write_var(unsigned char*& buff_ind, T const& var) noexcept {
		//list
		if constexpr (log_type_metadata_v<T> == ut_cast(aggregate_type::List)) {
			FSTLOG_ASSERT(reinterpret_cast<std::uintptr_t>(buff_ind) % constants::internal_msg_data_alignment == 0);
			const auto list_counter = static_cast<msg_counter>(var.size());
			FSTLOG_ASSERT(var.size() == list_counter);
			write_var(buff_ind, list_counter);
			for (const auto& v : var) {
				write_var(buff_ind, v);
			}
		}
		//tuple
		else if constexpr (log_type_metadata_v<T> == ut_cast(aggregate_type::Tuple)) {
			write_tuple(buff_ind, var, std::make_index_sequence<std::tuple_size_v<rm_cvref_t<T>>>{});
		}
		else {
			static_assert(!sizeof(T), "fstlog: write_var() Can not log Aggregate!");
		}
	}

	namespace {
		template<typename Tuple, std::size_t... Ints>
		void write_tuple(unsigned char*& buff_ind, Tuple const& tuple, std::index_sequence<Ints...>) noexcept {
			(write_var(buff_ind, std::get<Ints>(tuple)), ...);
		}
	}
}
