//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstdint>
#include <array>

#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/fast_to_str.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
	namespace detail {
		class nano_to_seconds_txt {
		public:
			nano_to_seconds_txt() noexcept = default;
			nano_to_seconds_txt(stamp_type timestamp, int second_char_num) noexcept
				: second_char_num_(second_char_num)
			{
				FSTLOG_ASSERT((second_char_num == 0
					|| second_char_num == 2
					|| (second_char_num >= 4 && second_char_num <= 12))
					&& "Invalid second_char_num value!");

				std::intmax_t ticks{ timestamp.time_since_epoch().count() };
				constexpr std::intmax_t ticks_in_minute{ (stamp_type::period::den * 60) / stamp_type::period::num };
				static_assert(ticks_in_minute > 0);
				std::intmax_t min{ ticks / ticks_in_minute };
				auto minutes{ std::chrono::minutes{static_cast<std::chrono::minutes::rep>(min)} };
				auto nanosecs_remain{
					std::chrono::duration_cast<std::chrono::nanoseconds>(
						std::chrono::system_clock::duration{ticks - (min * ticks_in_minute)}).count() };
				if (nanosecs_remain < 0) {
					nanosecs_remain += 60'000'000'000LL;
					minutes--;
				}
				
				FSTLOG_ASSERT(nanosecs_remain >= 0 && nanosecs_remain < 60'000'000'000LL);
				static_assert('9' < 127 && '0' < '9' && '0' + 1 == '1'
					&& '0' + 2 == '2' && '0' + 3 == '3' && '0' + 4 == '4'
					&& '0' + 5 == '5' && '0' + 6 == '6' && '0' + 7 == '7'
					&& '0' + 8 == '8' && '0' + 9 == '9');
				//set string and compute minutes
				if (second_char_num != 0) {
					to_dec(static_cast<std::uint64_t>(nanosecs_remain), sec_str_);
					const auto sec_pos{ sec_str_ + buffer_size - 11 };
					// rounding with "round half up" rule
					int dropped_digits = second_char_num == 2 ? 9 : 12 - second_char_num;
					if (dropped_digits > 0) {
						auto digit_ptr{ sec_str_ + buffer_size - dropped_digits };
						if (*digit_ptr >= '5') {
							while (++*--digit_ptr > '9') {
								*digit_ptr = '0';
							}
							//correcting round up to a minute
							if (*sec_pos != '6') {
								//dont have to round
							}
							else {
								*sec_pos = '0';
								++minutes;
							}
						}
					}

					*(sec_pos - 1) = *(sec_pos);
					*sec_pos = *(sec_pos + 1);
					*(sec_pos + 1) = '.';
				}
				//round to minutes
				else if (nanosecs_remain >= 30'000'000'000) {
					++minutes;
				}
								
				minutes_ = std::chrono::system_clock::time_point{ minutes };
			}

			std::string_view second_str() const noexcept {
				return { sec_str_ + buffer_size - 12, static_cast<std::size_t>(second_char_num_) };
			}

			stamp_type minutes() const noexcept {
				return minutes_;
			}

		private:
			static constexpr std::size_t buffer_size = 32;
			alignas(constants::cache_ls_nosharing) char sec_str_[buffer_size]{
				'0', '0', '0', '0', '0', '0', '0', '0',
				'0', '0', '0', '0', '0', '0', '0', '0',
				'0', '0', '0', '0', '0', '0', '0', '0',
				'0', '0', '0', '0', '0', '0', '0', '0' };
			stamp_type minutes_;
			int second_char_num_{ 0 };
		};
	}
}
