//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <chrono>
#include <limits>

#include <formatter/impl/detail/time_string.hpp>

namespace fstlog {
	template<int size_>
	class time_string_cache {
		using min_rep_type = std::chrono::minutes::rep;
	public:
		time_string_cache() noexcept {
			clear();
		}

		time_string<64>* find(min_rep_type key) noexcept {
			time_string<64>* out_ptr{ cached_strings_.data()};
			for (auto k : keys_) {
				if (k == key) {
					return out_ptr;
				}
				out_ptr++;
			}
			return nullptr;
		}
		time_string<64>& replace_last(
			time_string<64> time_str, 
			min_rep_type new_key) noexcept
		{
			time_string<64>& t_str{ cached_strings_[insert_index_] };
			t_str = time_str;
			keys_[insert_index_++] = new_key;
			if (insert_index_ == size_) 
				insert_index_ = 0;
			return t_str;
		}

		static constexpr int size() noexcept {
			return size_;
		}

		void clear() noexcept {
			keys_.fill((std::numeric_limits<min_rep_type>::min)());
			cached_strings_.fill(time_string<64>{});
			insert_index_ = 0;
		}
	private:
		std::array <min_rep_type, size_> keys_;
		int insert_index_;
		std::array<time_string<64>, size_> cached_strings_;
	};
}
