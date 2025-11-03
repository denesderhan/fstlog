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
        filter_internal() noexcept = default;

        filter_internal(
            level lowest,
            channel_type channel) noexcept
        {
            add_level(level::Fatal, lowest);
            add_channel(channel);
        }

        filter_internal(
            level lowest,
            channel_type first_channel,
            channel_type last_channel) noexcept 
        {
            add_level(level::Fatal, lowest);
            add_channel(first_channel, last_channel);
        }

        bool operator==(const filter_internal& other) const noexcept {
            return level_data_ == other.level_data_
                && channel_data_ == other.channel_data_;
        }

        bool operator!=(const filter_internal& other) const noexcept {
            return !(*this == other);
        }

        void add_level(level level) noexcept {
            FSTLOG_ASSERT(level <= fstlog::level::All);
            level_data_ = level_data_ | (std::uint32_t{ 1 } << ut_cast(level));
        }

        void add_level(level first, level last) noexcept {
            auto lvl = first < last ? first : last;
            const auto last_lvl = last > first ? last : first;
            while (true) {
                add_level(lvl);
                if (lvl == last_lvl) break;
                lvl = level(ut_cast(lvl) + 1);
            }
        }

        void add_channel(channel_type channel) noexcept {
            const auto index{ channel / 32 };
            const auto bit_pos{ channel & 31 };
            channel_data_[index] |= (std::uint32_t{ 1 } << bit_pos);
        }

        void add_channel(channel_type first, channel_type last) noexcept {
            auto ch = first < last ? first : last;
            const auto last_ch = last > first ? last : first;
            while (true) {
                add_channel(ch);
                if (ch == last_ch) break;
                ch++;
            }
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
        
        std::uint32_t level_data_{ 0 };
        std::array<std::uint32_t, 8> channel_data_{ 0, 0, 0, 0, 0, 0, 0, 0 };

        static_assert(sizeof(level_data_) * 8 > ut_cast(fstlog::level::All));
        static_assert((std::numeric_limits<channel_type>::max)() <= 255);
        static_assert((std::numeric_limits<channel_type>::min)() == 0);
    };
}
