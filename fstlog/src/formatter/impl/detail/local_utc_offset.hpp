//Copyright © 2025, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <ctime>
#ifndef _WIN32
#include <ctime>
#endif

namespace fstlog::detail {
    // ISO 8601 offset from UTC
    struct utc_offset {
        std::array<char, 6> standard{ '+', '0', '0', '0', '0', 0 };
        std::array<char, 6> daylight{ '+', '0', '0', '0', '0', 0 };
    };

    namespace {

        constexpr int compile_year() noexcept {
            char d[] = __DATE__;
            static_assert(sizeof(__DATE__) >= 5, "No year in __DATE__?");
            const auto ind = sizeof(__DATE__) - 5;
            int year =
                (d[ind] - '0') * 1000
                + (d[ind + 1] - '0') * 100
                + (d[ind + 2] - '0') * 10
                + (d[ind + 3] - '0');
            return year;
        }

        inline void format_utc_offset_string(int offset_minutes, std::array<char, 6> &buffer) noexcept {
            if (offset_minutes < 0) {
                buffer[0] = '-';
                offset_minutes = -offset_minutes;
            }
            else{
                buffer[0] = '+';
            }
            
            buffer[5] = 0;
            int temp = (offset_minutes / 60) * 100 + offset_minutes % 60;
            for (int i = 4; i > 0; i--) {
                buffer[i] = '0' + temp % 10;
                temp /= 10;
            }
        }
    }

    // preformats the "+0000" ISO 8601 UTC offset strings used for timestamp formatting
    // according to the current locale information
    // utc_offset contains one string for DST active and one for inactive case
    inline utc_offset get_utc_offset() noexcept {
        utc_offset result{};
        std::tm time{};
        
        constexpr int fallback_year = compile_year() < 2025 ? 125 : compile_year() - 1900;

        auto unix_time = std::time(nullptr);
        if (unix_time >= 0) {
    #ifdef _WIN32
            errno_t error = localtime_s(&time, &unix_time);
            if (error != 0) time.tm_year = fallback_year;
    #else
            tm* error = localtime_r(&unix_time, &time);
            if (error == nullptr) time.tm_year = fallback_year;
    #endif
        }
        else {
            time.tm_year = fallback_year;
        }

        // USA Second Sunday in March (at least 8 days into March)
        // Europe Last Sunday in March (mutch more days into March)
        time.tm_mday = 5; // set for catching the change early (March 5. not in DST, April 5. in DST)
        time.tm_hour = 12; // Avoiding the ambiguous time period of clock adjustment in early morning.
        time.tm_min = 0;
        time.tm_sec = 0;

        // 24 hours offset, an impossible value
        const int non_init = 24 * 60;
        int standard_offset = non_init;
        int daylight_offset = non_init;

        // starting from the month march (early exit in most time zones)
        for (int i = 2; i < 14; i++) {
            // changing the month (the year remains the same)
            time.tm_mon = i < 12 ? i : i - 12;
                        
            const std::time_t utc_time =
    #ifdef _WIN32            
                _mkgmtime(&time);
    #else
                timegm(&time);
    #endif
            if (utc_time == -1) continue; // skip month on error
            
            time.tm_isdst = -1; // querying DST
            std::time_t local_time = std::mktime(&time);
            if (local_time == -1) continue; // skip month on error

            // compute the minute offset (if not in range set to non_init)
            const double max_diff = 23.0 * 60.0; // 23 hours (14 hours currently in practice)
            auto diff = std::difftime(utc_time, local_time) / 60.0;
            int minute_offset{ 0 }; 
            if (diff > max_diff    || diff < -max_diff) {
                minute_offset = non_init;
            }
            else {
                minute_offset = static_cast<int>(diff);
            }
            
            // DST is active
            if (time.tm_isdst > 0) {
                daylight_offset = minute_offset;
            }
            // DST is inactive or no DST
            else {
                standard_offset = minute_offset;
            }

            // if we found both offsets, break
            if (daylight_offset != non_init && standard_offset != non_init) {
                break;
            }
        }

        // setting uninitialized offsets
        if (daylight_offset == non_init && standard_offset == non_init) {
            daylight_offset = 0;
            standard_offset = 0;
        }
        else if (daylight_offset == non_init) {
            daylight_offset = standard_offset;
        }
        else if (standard_offset == non_init) {
            standard_offset = daylight_offset;
        }

        // formatting strings
        format_utc_offset_string(standard_offset, result.standard);
        format_utc_offset_string(daylight_offset, result.daylight);
        
        return result;
    }
}
