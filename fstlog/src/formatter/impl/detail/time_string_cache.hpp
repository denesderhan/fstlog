//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <chrono>
#include <limits>

#include <fstlog/detail/small_string.hpp>

namespace fstlog {
	template<int size_>
	class time_string_cache {
		using min_type = std::chrono::minutes::rep;
	public:
		time_string_cache() noexcept {
			clear();
		}

		small_string<64> find(min_type key) noexcept {
			for (auto const &p : cache_) {
				if (p.second == key) {
					return p.first;
				}
			}
			return small_string<64>{};
		}
		
		void replace_oldest(
			small_string<64> const &time_str, 
			min_type new_key) noexcept
		{
			cache_[insert_index_++] = { time_str, new_key };
			if (insert_index_ == size_) insert_index_ = 0;
		}

		static constexpr int size() noexcept {
			return size_;
		}

		void clear() noexcept {
			cache_.fill({ small_string<64>{}, (std::numeric_limits<min_type>::min)() });
			insert_index_ = 0;
		}
	private:
		int insert_index_;
		std::array<std::pair<small_string<64>, min_type>, size_> cache_;
	};
}
