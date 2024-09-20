//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <limits>

#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/level.hpp>
#include <fstlog/detail/types.hpp>
#include <fstlog/detail/ut_cast.hpp>

namespace fstlog {
	class filter_internal {
	public:
		constexpr filter_internal() noexcept = default;
		constexpr filter_internal(
			level lowest,
			channel_type channel) noexcept
			: level_data_{ make_level_data(level::Fatal, lowest) },
			channel_data_{ make_channel_data(channel, channel) } {}
		constexpr filter_internal(
			level lowest,
			channel_type first_channel,
			channel_type last_channel) noexcept
			: level_data_{ make_level_data(level::Fatal, lowest) },
			channel_data_{ make_channel_data(first_channel, last_channel) } {}

		void add_level(level level) noexcept {
			FSTLOG_ASSERT(level <= fstlog::level::All);
			level_data_ = level_data_ | (std::uint32_t{ 1 } << ut_cast(level));
		}
		void add_level(level first, level last) noexcept {
			const auto data = make_level_data(first, last);
			level_data_ |= data;
		}
		void add_channel(channel_type channel) noexcept {
			const auto index{ channel / 32 };
			const auto bit_pos{ channel & 31 };
			channel_data_[index] |= (std::uint32_t{ 1 } << bit_pos);
		}
		void add_channel(channel_type first, channel_type last) noexcept {
			const auto data = make_channel_data(first, last);
			for (int i = 0; i < 8; i++)
				channel_data_[i] |= data[i];
		}
		bool filter_msg(level level, channel_type channel) const noexcept {
			if ((std::uint32_t{ 1 } << ut_cast(level)) & level_data_) {
				const auto index{ channel / 32 };
				const auto bit_pos{ channel & 31 };
				return channel_data_[index] & (std::uint32_t{ 1 } << bit_pos);
			}
			else return false;
		}

private:
		static constexpr std::uint32_t make_level_data(level first, level last) noexcept {
			const auto f = first < last ? first : last;
			const auto l = last > first ? last : first;
			std::uint32_t out{ 0 };
			for (auto lvl = ut_cast(f); lvl <= ut_cast(l); lvl++) {
				out |= (std::uint32_t{ 1 } << lvl);
			}
			return out;
		}

		static constexpr std::array<std::uint32_t, 8> make_channel_data(channel_type first, channel_type last) noexcept {
			std::array<std::uint32_t, 8> out{ 0 };
			const auto f = first < last ? first : last;
			const auto l = last > first ? last : first;

			channel_type channel{ 0 };
			for (int ind = 0; ind < 8; ind++) {
				auto b{ ind * 32 };
				auto e{ b + 31 };
				if ((f > b && f <= e)
					|| (l >= b && l < e))
				{
					for (int i = 0; i < 32; i++) {
						if (channel >= f && channel <= l) {
							const auto bit_pos{ channel & 31 };
							out[ind] |= std::uint32_t{ 1 } << bit_pos;
						}
						channel++;
					}
				}
				else if (f <= b && l >= e) {
					out[ind] = 0xffffffff;
					channel += 32;
				}
				else {
					channel += 32;
				}
			}
			return out;
		}

		std::uint32_t level_data_{ 0 };
		std::array<std::uint32_t, 8> channel_data_{ 0, 0, 0, 0, 0, 0, 0, 0 };

		static_assert(sizeof(level_data_) 
			* std::numeric_limits<unsigned char>::digits > ut_cast(fstlog::level::All));
		static_assert((std::numeric_limits<channel_type>::max)() <= 255);
		static_assert((std::numeric_limits<channel_type>::min)() == 0);
		static_assert(
			sizeof(decltype(channel_data_)::value_type) 
				* std::tuple_size_v<decltype(channel_data_)> 
				* std::numeric_limits<unsigned char>::digits 
			>= (std::numeric_limits<channel_type>::max)() + 1);
	};
}
