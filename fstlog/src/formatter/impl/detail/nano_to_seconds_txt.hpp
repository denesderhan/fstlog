//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstdint>
#include <array>

#include <fstlog/detail/constants.hpp>
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
				FSTLOG_ASSERT(timestamp >= std::chrono::system_clock::time_point{});
				// set the rounded minute and second string
				decompose_timestamp(timestamp);
			}

			std::string_view second_str() const noexcept {
				return { sec_str_.data(), static_cast<std::size_t>(second_char_num_) };
			}

			stamp_type minutes() const noexcept {
				return std::chrono::system_clock::time_point(
					std::chrono::duration_cast<std::chrono::system_clock::duration>(
						std::chrono::minutes(minutes_)));
			}

		private:
			static constexpr long long nano_minute = 60'000'000'000LL;
			static constexpr std::intmax_t ticks_minute{ 
				(stamp_type::period::den * 60) / stamp_type::period::num };
			static_assert(ticks_minute > 0, "Tick, nano error.");
			static_assert(ticks_minute <= nano_minute 
				&& nano_minute % ticks_minute == 0, "Tick, nano error.");

			inline void decompose_timestamp(std::chrono::system_clock::time_point timestamp)
			{
				constexpr std::array<long long, 11> round_ticks{
						ticks_minute / 2LL,
						ticks_minute / 120LL,
						ticks_minute / 1'200LL,
						ticks_minute / 12'000LL,
						ticks_minute / 120'000LL,
						ticks_minute / 1'200'000LL,
						ticks_minute / 12'000'000LL,
						ticks_minute / 120'000'000LL,
						ticks_minute / 1'200'000'000LL,
						ticks_minute / 12'000'000'000LL };
				int round_ind = 0;
				if (second_char_num_ > 3) round_ind = second_char_num_ - 2;
				else if (second_char_num_ == 2) round_ind = 1;
				FSTLOG_ASSERT(round_ind >= 0 && round_ind <= 10);
				const long long rounder = round_ticks[round_ind];
				
				std::intmax_t ticks{ timestamp.time_since_epoch().count() };
				// preventing overflow
				if (ticks <= (std::numeric_limits<std::intmax_t>::max)() - (ticks_minute / 2LL)) {
					ticks += rounder; // round to nearest
				}
				// floor minutes
				minutes_ = ticks / ticks_minute;
				if (second_char_num_ != 0) {
					set_second(ticks);
				}
			}

			inline void set_second(std::intmax_t ticks) {
				// compute remainder
				ticks -= minutes_ * ticks_minute;
				FSTLOG_ASSERT(ticks >= 0LL && ticks < ticks_minute);
				std::intmax_t nano_ticks = ticks * (nano_minute / ticks_minute);
				
				// digit conversion table
				const char digits2[]{
					"0001020304050607080910111213141516171819"
					"2021222324252627282930313233343536373839"
					"4041424344454647484950515253545556575859"
					"6061626364656667686970717273747576777879"
					"8081828384858687888990919293949596979899" };
				int char_ind = 10;
				// converting digits, two at a time
				while (nano_ticks != 0) {
					const long long next_nano = nano_ticks / 100;
					const long long fraq = nano_ticks - next_nano * 100;
					const long long digit_ind = fraq * 2;
					sec_str_[char_ind] = digits2[digit_ind];
					sec_str_[char_ind + 1] = digits2[digit_ind + 1];
					char_ind -= 2;
					nano_ticks = next_nano;
				}
				// inserting decimal point
				sec_str_[0] = sec_str_[1];
				sec_str_[1] = sec_str_[2];
				sec_str_[2] = '.';
			}

			alignas(constants::cache_ls_nosharing) std::array<char, 12> sec_str_{
				'0','0','0','0','0','0','0','0','0','0','0','0' };
			std::intmax_t minutes_{ 0 };
			int second_char_num_{ 0 };
		};
	}
}
