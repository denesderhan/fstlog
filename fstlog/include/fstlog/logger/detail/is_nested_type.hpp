//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <type_traits>

#include <fstlog/detail/type_signature.hpp> 

namespace fstlog {
	template<typename T>
	struct is_nested_type {
		static constexpr bool value = [] {
			constexpr std::size_t signature_length = type_signature_v<T>.size();
			constexpr unsigned char tuple = ut_cast(log_element_type::Aggregate) | ut_cast(aggregate_type::Tuple);

			std::size_t aggr_num{ 0 };
			std::size_t index = 0;
			while (index < signature_length) {
				auto current_el = type_signature_v<T>[index++];
				if ((current_el & log_element_type_bitmask) == ut_cast(log_element_type::Aggregate)) {
					if (aggr_num == 1) return true;
					else aggr_num++;
					//skippinng tuple element counter
					if (current_el == tuple) index++;
				}
			}
			return false;
		}();
	};
}
