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
				FSTLOG_ASSERT(timestamp >= std::chrono::system_clock::time_point{} );

				int second_precision = 6;
				if (second_char_num == 0) second_precision = -1;
				else if (second_char_num == 2) second_precision = 0;
				else second_precision = second_char_num - 3;
				// get the rounded minute and second parts
				auto [minutes, seconds] = decompose_timestamp(timestamp, second_precision);
				minutes_ = minutes;
				auto nanosecs_remain = std::chrono::duration_cast<std::chrono::nanoseconds>(seconds).count();
				// set second string
				if (second_char_num != 0) {
					to_dec(static_cast<std::uint64_t>(nanosecs_remain), sec_str_);
					const auto sec_pos{ sec_str_ + buffer_size - 11 };
					*(sec_pos - 1) = *(sec_pos);
					*sec_pos = *(sec_pos + 1);
					*(sec_pos + 1) = '.';
				}
			}

			std::string_view second_str() const noexcept {
				return { sec_str_ + buffer_size - 12, static_cast<std::size_t>(second_char_num_) };
			}

			stamp_type minutes() const noexcept {
				return minutes_;
			}

		private:
			
			std::pair<std::chrono::system_clock::time_point, std::chrono::system_clock::duration>
				decompose_timestamp(std::chrono::system_clock::time_point timestamp, int second_precision)
			{
				constexpr long long minute_in_nano = 60'000'000'000LL;
				std::array<long long, 11> lut_nano{
					60'000'000'000LL,
					1'000'000'000LL,
					100'000'000LL,
					10'000'000LL,
					1'000'000LL,
					100'000LL,
					10'000LL,
					1'000LL,
					100LL,
					10LL,
					1LL };

				FSTLOG_ASSERT(second_precision >= -1 && second_precision <= 9);
				long long unit = lut_nano[second_precision + 1];
				long long half_unit = unit / 2;
				long long nano_epoch = 
					std::chrono::duration_cast<std::chrono::nanoseconds>(
						timestamp.time_since_epoch()).count();
				long long count = 0LL;
				// preventing overflow
				if (nano_epoch <= (std::numeric_limits<long long>::max)() - half_unit) {
					count = (nano_epoch + half_unit) / unit; // round to nearest
				}
				else {
					count = nano_epoch / unit; // round down
				}
				long long rounded_nano = count * unit;
				// floor minutes
				long long minutes_part = rounded_nano / minute_in_nano;
				long long seconds_part = rounded_nano - minutes_part * minute_in_nano;
				return {
					std::chrono::system_clock::time_point(
						std::chrono::duration_cast<std::chrono::system_clock::duration>(
							std::chrono::minutes(minutes_part))),
					std::chrono::duration_cast<std::chrono::system_clock::duration>(
						std::chrono::nanoseconds(seconds_part)) };
			}

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
