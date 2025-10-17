//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <time.h>
#include <type_traits>

#include <detail/unaligned_span.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <detail/utf_conv.hpp>
#include <detail/utf8_len.hpp>
#include <formatter/impl/detail/format_setting_txt.hpp>
#include <formatter/impl/detail/format_str_helper.hpp>
#include <formatter/impl/detail/local_utc_offset.hpp>
#include <formatter/impl/detail/shift_fill.hpp>
#include <formatter/impl/detail/time_string_cache.hpp>
#include <formatter/impl/detail/tz_format.hpp>
#include <formatter/impl/detail/valid_strftime_string.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
    template<bool use_fill_align, typename L>
    class encoder_timestamp_mixin : public L {
    public:
        using memory_resource_type = typename L::memory_resource_type;

        explicit encoder_timestamp_mixin(memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<L, memory_resource_type*>)
            : L(resource) {}

        encoder_timestamp_mixin(const encoder_timestamp_mixin& other) noexcept(
            std::is_nothrow_constructible_v<
                encoder_timestamp_mixin,
                const encoder_timestamp_mixin&,
                memory_resource_type*>)
            : encoder_timestamp_mixin(other, other.get_memory_resource()) {}
        encoder_timestamp_mixin(const encoder_timestamp_mixin& other, memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<
                L,
                const L&,
                memory_resource_type*>)
            : L(static_cast<const L&>(other), resource),
            time_string_cache_{ other.time_string_cache_ },
            time_format_{ other.time_format_ },
            tzone_{ other.tzone_ },
            formatted_length_{ other.formatted_length_ },
            second_precision_{ other.second_precision_ },
            second_pos_{ other.second_pos_ } {}

        encoder_timestamp_mixin(encoder_timestamp_mixin&& other) = delete;
        encoder_timestamp_mixin& operator=(const encoder_timestamp_mixin& rhs) = delete;
        encoder_timestamp_mixin& operator=(encoder_timestamp_mixin&& rhs) = delete;

        ~encoder_timestamp_mixin() = default;

        /**
         * @brief Initializes the timestamp encoder with the specified format string.
         *
         * @details
         * - Checks if the encoder has already been initialized.
         * - Decomposes the format string to extract fill, alignment, width, and precision settings.
         * - Sets the internal time format string and validates it.
         * - Applies fill and alignment to the time format string if required.
         * - Prepares the seconds placeholder for efficient timestamp encoding.
         *
         * @param time_format The format string for timestamp encoding (byte_span_const).
         * @return error_code
         *     - error_code::none on success.
         *     - error_code::double_init if already initialized.
         *     - error_code::fmt_bad if the format string is invalid.
         *     - error_code::str_long if the format string is too long.
         *
         * @note
         * - This function should be called only once during encoder setup.
         * - Internal state is updated to reflect the chosen format and settings.
         * - Assumes that all dependencies and member variables are properly initialized.
         *
         * @see decompose_format
         * @see set_time_format
         * @see apply_fill_align
         * @see prepare_seconds_placeholder
         */
        error_code init_encoder_timestamp(byte_span_const time_format) noexcept {
            if (!time_format_.empty()) {
                return error_code::double_init;
            }

            format_setting_txt format{};
            auto error = decompose_format(time_format, format);
            if (error != error_code::none) return error;
            // try to set time_format_
            error = set_time_format(time_format);
            if (error != error_code::none) return error;
                        
            // we append the fill characters to the time_format_ string 
            // this is safe because the formatted length is constant
            error = apply_fill_align(format);
            if (error != error_code::none) return error;
            
            prepare_seconds_placeholder();

            return error_code::none;
        }

        /**
         * @brief Encodes a timestamp into the output buffer using the configured format.
         *
         * @details
         * - Validates the input timestamp and output buffer space.
         * - Rounds the timestamp down to the nearest minute and extracts nanoseconds.
         * - Attempts to retrieve a cached formatted time string for the minute.
         * - If not cached, creates and stores the formatted time string.
         * - Copies the formatted time string to the output buffer.
         * - Replaces the seconds placeholder with the actual seconds value.
         * - Advances the output buffer pointer.
         *
         * @param timestamp The timestamp to encode (stamp_type).
         *
         * @note
         * - Sets error state if the timestamp is invalid or buffer is full.
         * - Uses internal cache for efficient repeated formatting.
         * - Assumes output buffer and format have been properly initialized.
         */
        void encode_timestamp(stamp_type timestamp) noexcept {
            if (timestamp < 0) {
                this->set_error(__FILE__, __LINE__, error_code::input_bad);
                return;
            }
            if (!this->output_has_space(formatted_length_)) {
                this->set_error(__FILE__, __LINE__, error_code::buff_full);
                return;
            }

            const stamp_type minutes{ timestamp / 60'000'000'000LL };
            const stamp_type nanoseconds{ timestamp - minutes * 60'000'000'000LL };
            
            static_assert((std::numeric_limits<stamp_type>::max)() / 60'000'000'000LL
                <= (std::numeric_limits<std::int32_t>::max)(), "stamp_type too big, key can overflow!");
            const auto key{ static_cast<std::int32_t>(minutes) };
            // search the minute in the cache
            auto time_string{ time_string_cache_.find(key) };
            // if not found create and store
            if (time_string.empty()) {
                time_string = create_time_string(minutes);
                FSTLOG_ASSERT(time_string.size() == formatted_length_);
                time_string_cache_.replace_oldest(time_string, key);
            }

            const auto out_begin = this->output_ptr();
            std::memcpy(out_begin, time_string.data(), formatted_length_);
            // replacing the second placeholder with the second string
            write_second(nanoseconds, out_begin);
            
            this->advance_output(formatted_length_);
        }

    private:

        /**
        * @brief Decomposes the format string for timestamp encoding.
        *
        * @details
        * - Parses the input format string to extract fill characters, alignment, width, and precision.
        * - Updates the format_setting_txt structure with parsed values.
        * - Sets the second_precision_ member to control the precision of seconds in the timestamp.
        * - Modifies the input time_format span to point to the actual time format substring.
        *
        * @param[in,out] time_format Reference to the format string span; updated to exclude fill/align/width/precision.
        * @param[out] format Structure to receive fill, align, width, and precision settings.
        * @return error_code
        *     - error_code::none on success.
        *     - error_code::fmt_bad if the format string is invalid.
        *
        * @note
        * - Only called during initialization of the timestamp encoder.
        * - Uses helper functions to skip and parse fill/align, sign, width, and precision.
        * - Limits second precision to a maximum of 9.
        */
        error_code decompose_format(
            byte_span_const& time_format,
            format_setting_txt& format) noexcept
        {
            auto temp = time_format;
            time_format = skip_fill_align(time_format);
            if constexpr (use_fill_align) {
                auto fill_begin = temp.data_bytes();
                auto align_end = time_format.data_bytes();
                if (fill_begin < align_end) {
                    format.align = *--align_end;
                }
                const std::size_t utf8_seq_len = static_cast<std::size_t>(align_end - fill_begin);
                FSTLOG_ASSERT(utf8_seq_len <= format.fill_char.size());
                std::memcpy(format.fill_char.data(), fill_begin, utf8_seq_len);
            }
            temp = skip_sign_alt_0(time_format);
            // usage of sign, alt '0' is an error
            if (temp.data_bytes() != time_format.data_bytes()) {
                return error_code::fmt_bad;
            }

            format.width = get_width(time_format);
            // precision is the precision of seconds (precision 0: "30" precision 2 : "30.44")
            auto precision = get_precision(time_format);
            if (precision > 9) precision = 9;
            second_precision_ = static_cast<unsigned char>(precision);
            return error_code::none;
        }

        
        /**
        * @brief Sets the time format string for timestamp encoding.
        *
        * @details
        * - Determines the timezone from the format string.
        * - If no format string is provided, sets a default format based on the timezone.
        * - Validates the format string using valid_strftime_string.
        * - Preprocesses the format string to handle placeholders and timezone.
        * - Attempts to create a sample timestamp string to determine the formatted length.
        * - Returns an error if the format string is too long or invalid.
        *
        * @param format_string The input format string as a byte span.
        * @return error_code
        *     - error_code::none on success.
        *     - error_code::str_long if the format string is too long.
        *     - error_code::fmt_bad if the format string is invalid.
        *
        * @note
        * - This function is called during initialization of the timestamp encoder.
        * - The formatted string size is constant; variable length strftime specifiers are not allowed.
        */
        error_code set_time_format(byte_span_const format_string) noexcept {
            tzone_ = get_zone(format_string);
            
            // no strftime string, set default
            if (format_string.empty()) {
                std::string_view default_format = "%Y-%m-%d %H:%M:%S %z";
                format_string = byte_span_const{
                    safe_reinterpret_cast<const unsigned char*>(default_format.data()),
                    default_format.size()};
            }
            
            if(format_string.size() > small_string<64>::capacity()) {
                return error_code::str_long;
            }

            if (!detail::valid_strftime_string(format_string)) {
                return error_code::fmt_bad;
            }

            time_format_ = small_string<64>(
                safe_reinterpret_cast<const char*>(format_string.data_bytes())
                , format_string.size());

            auto error = preformat_time_format();
            if (error != error_code::none) return error;

            // try to format a timestamp ( without replacing the seconds placeholder )
            auto stamp_str = create_time_string(stamp_type{});
            // if empty, there was not enough space (string size can grow do to formatting)
            if (stamp_str.empty()) {
                return error_code::str_long;
            }
            // the formatted string size will be a constant ( no variable length strftime format specifiers are allowed)
            formatted_length_ = static_cast<unsigned char>(stamp_str.size());
                                    
            return error_code::none;
        }

        /**
         * @brief Prepares the timestamp format by finding and updating the seconds placeholder
         * 
         * @details This function performs two main tasks:
         * 1. Locates the position of the seconds placeholder (marked by 0x1 bytes) in the formatted timestamp
         * 2. Updates the format string by replacing all 0x1 placeholder bytes with '0' characters
         * 
         * The function first creates a sample timestamp string to find where the seconds should be.
         * It searches for the first occurrence of the value 1 (0x1) which marks the seconds position.
         * If no seconds placeholder is found, sets second_pos_ to 255 (invalid position).
         * Finally replaces all placeholder bytes (0x1) in time_format_ with '0' characters.
         * 
         * @note This function is called during initialization after the time format string has been set
         * and preprocessed by preformat_time_format(). The seconds position is later used by write_second()
         * to efficiently insert the actual seconds value.
         * 
         * @warning The function assumes that time_format_ has been properly initialized and preprocessed
         * 
         * @post The second_pos_ member will be set to either:
         *       - The byte offset of the seconds field
         *       - 255 if no seconds field exists in the format
         * @post All 0x1 bytes in time_format_ will be replaced with '0' characters
         */
        void prepare_seconds_placeholder() noexcept {
            auto stamp_str = create_time_string(stamp_type{});
            std::size_t sec_pos = 0;
            while (sec_pos < stamp_str.size() && *(stamp_str.data() + sec_pos) != 1) sec_pos++;
            if (sec_pos == stamp_str.size()) sec_pos = 255;
            second_pos_ = static_cast<unsigned char>(sec_pos);
            
            // replace 0x1-s with '0'-s
            std::array<char, 64> temp{ 0 };
            std::memcpy(temp.data(), time_format_.data(), time_format_.size());
            for (auto& c : temp) if (c == 1) c = '0';
            time_format_ = small_string<64>(temp.data(), time_format_.size());
        }

        /**
         * @brief Applies fill and alignment to the time format string for timestamp encoding.
         *
         * @details
         * - Computes the formatted string length for the timestamp.
         * - If the formatted length is less than the specified width, attempts to append fill characters.
         * - Uses the formatted character count rather than the format string character count for fill.
         * - Updates the internal time_format_ string and formatted_length_ if fill is applied.
         * - Returns error_code::str_long if the new length exceeds the capacity.
         *
         * @param format The format settings including fill character, alignment, and width.
         * @return error_code
         *     - error_code::none on success.
         *     - error_code::str_long if the filled string exceeds capacity.
         *
         * @note
         * This function is noexcept and modifies internal state.
         */
        error_code apply_fill_align(format_setting_txt format) noexcept {
            if constexpr (use_fill_align) {
                // compute string lengths for the formatted timestamp
                const auto stamp_str = create_time_string(stamp_type{});
                const detail::utf8_len formatted_len =
                    detail::utf::utf8_str_trim(unaligned_span{ stamp_str.data(), stamp_str.size() });

                // we can only do fill-align if length < width
                if (formatted_len.char_len < format.width) {
                    const std::size_t time_format_bytes = time_format_.size();
                    std::array<unsigned char, small_string<64>::capacity()> temp{};
                    std::memcpy(temp.data(), time_format_.data(), time_format_bytes);
                    // try to add fill characters to the time_format_ 
                    // but use the formatted character number instead of the format string character number
                    const detail::utf8_len result_len = detail::shift_fill(
                        temp,
                        { time_format_bytes, formatted_len.char_len },
                        format);
                    // replace time_format_ if appending succeded
                    if (result_len.byte_len > time_format_bytes) {
                        std::size_t new_length = formatted_length_ + (result_len.byte_len - time_format_bytes);
                        if (new_length > time_format_.capacity()) {
                            return error_code::str_long;
                        }
                        formatted_length_ = static_cast<unsigned char>(new_length);
                        time_format_ = small_string<64>(
                            safe_reinterpret_cast<const char*>(temp.data()), result_len.byte_len);
                    }
                }
            }
            return error_code::none;
        }
                
        /**
        * @brief Preformats the time_format_ string for timestamp encoding.
        *
        * @details
        * - Replaces %S with a placeholder (0x1 bytes) and sets the position for seconds.
        * - Replaces %z with "+0000" if the timezone is UTC.
        * - Copies other characters as-is.
        * - Updates the internal time_format_ string.
        * - Returns error_code::str_long if there is not enough space in the output buffer.
        *
        * @return error_code
        *     - error_code::none on success.
        *     - error_code::str_long if output buffer is too small.
        *
        * @note
        * This function is noexcept and modifies internal state.
        */
        error_code preformat_time_format() noexcept {
            unaligned_span<const char> input{
                time_format_.data(),
                time_format_.size() };
            std::array<char, decltype(time_format_)::capacity()> temp{0};
            unaligned_span<char> output{
                temp.data(),
                temp.size() };
            while (input.size() > 1) {
                // replace %S conversion specifier
                if (input.template get<0>() == '%' 
                    && input.template get<1>() == 'S')
                {
                    second_pos_ = static_cast<unsigned char>(temp.size() - output.size());
                    const std::size_t second_charnum = second_precision_ == 0 ? 2 : second_precision_ + 3;
                    if(output.size() < second_charnum) {
                        return error_code::str_long; // not enough space
                    }
                    std::memset(output.data_bytes(), 1, second_charnum);
                    output.drop_front(second_charnum);
                    input.template drop_front<2>();
                }
                // replace %z conversion specifier only if tzone_ is UTC
                else if (input.template get<0>() == '%'
                    && input.template get<1>() == 'z'
                    && tzone_ == tz_format::UTC)
                {
                    if (output.size() < 5) {
                        return error_code::str_long; // not enough space
                    }
                    std::memcpy(output.data_bytes(), "+0000", 5);
                    output.template drop_front<5>();
                    input.template drop_front<2>();
                }
                else {
                    if (output.empty()){
                        return error_code::str_long; // not enough space
                    }
                    output.template set<0>(input. template get<0>());
                    input.template drop_front<1>();
                    output.template drop_front<1>();
                }
            }
            // copy the remaining character if any
            if(!input.empty()) {
                if(output.empty()){
                    return error_code::str_long; // not enough space
                }
                else {
                    output.template set<0>(input.template get<0>());
                    output.template drop_front<1>();
                }
            }
            time_format_ = small_string<64>(temp.data(), temp.size() - output.size());
            return error_code::none;
        }

        /**
         * @brief Creates a formatted time string with "00....0" second placeholder for the given timestamp.
         *
         * @details
         * - Converts the input timestamp to a time_t value.
         * - Retrieves the corresponding tm structure in either UTC or local time, based on tzone_.
         * - Returns an empty small_string<64> if conversion fails.
         * - Calls format_time() to generate the formatted time string.
         *
         * @param minutes The timestamp (rounded to minutes) to format.
         * @return small_string<64> The formatted time string, or empty on error/not enough space.
         *
         * @note
         * - The seconds field is not filled in the returned string.
         * - Handles platform-specific differences for time conversion (Windows vs POSIX).
         * - Used internally for timestamp formatting and caching.
         */
        small_string<64> create_time_string(stamp_type minutes) noexcept {
            time_t const t{ static_cast<time_t>(minutes * 60) };
            tm time;
        #ifdef _WIN32
            errno_t error{};
            if (tzone_ == tz_format::UTC) error = gmtime_s(&time, &t);
            else error = localtime_s(&time, &t);
            if (error != 0) return small_string<64>{};
        #else        
            tm* error{};
            if (tzone_ == tz_format::UTC) error = gmtime_r(&t, &time);
            else error = localtime_r(&t, &time);
            if (error == nullptr) return small_string<64>{};
        #endif
            return format_time(time);
        }

        /**
         * @brief Formats a time structure into a timestamp string using the configured format.
         *
         * @details
         * - Uses the internal time_format_ string to format the provided std::tm structure.
         * - Supports a subset of strftime specifiers: %% (percent), %Y (year), %m (month), %d (day),
         *   %H (hour), %M (minute), %z (UTC offset), %y (2-digit year), %a (abbreviated weekday).
         * - Unsupported specifiers are copied as-is with a leading '%'.
         * - Numeric fields are zero-padded to two digits.
         * - The output is written to a fixed-size buffer and returned as a small_string<64>.
         * - If the formatted string exceeds the buffer capacity, returns an empty small_string<64>.
         *
         * @param[in] time The std::tm structure representing the time to format.
         * @return small_string<64> The formatted timestamp string, or empty if formatting fails.
         *
         * @note
         * - The function assumes time_format_ is properly initialized and non-empty.
         * - The output buffer size is statically checked to be sufficient for worst-case expansion.
         * - Used internally for timestamp encoding and caching.
         *
         * @see small_string
         * @see std::tm
         */
        small_string<64> format_time(std::tm const& time) {
            if (time_format_.empty()) return small_string<64>();
            std::array<char, 160> buff{ 0 };
            // proving that the length can not grow out of the buffer
            static_assert(
                (decltype(time_format_)::capacity() / 2) * 5 // worst case for length grow (%z 5)
                + 1 // upper bound in case of odd length
                + 1 // 0 at end (to be safe)
                < buff.size(), "buff small");
            const auto form_str_len = time_format_.size();
            std::size_t form_pos = 0;
            std::size_t pos = 0;
            while (form_pos < form_str_len - 1) {
                char c0 = *(time_format_.data() + form_pos++);
                // not a format specifier
                if (c0 != '%') {
                    buff[pos++] = c0;
                    continue;
                }
                
                int num{ 0 };
                const char c1 = *(time_format_.data() + form_pos++);
                // %% (%)
                if (c1 == '%') {
                    buff[pos++] = '%';
                }
                // %Y (year 2025)
                else if (c1 == 'Y') {
                    std::size_t year = 1900;
                    year += time.tm_year;
                    buff[pos + 3] = '0' + year % 10; year /= 10;
                    buff[pos + 2] = '0' + year % 10; year /= 10;
                    buff[pos + 1] = '0' + year % 10; year /= 10;
                    buff[pos] = '0' + year % 10;
                    pos += 4;
                }
                // %m (month 01-12)
                else if (c1 == 'm') {
                    num = time.tm_mon + 1;
                    c0 = 0;
                }
                // %d (day 01-31)
                else if (c1 == 'd') {
                    num = time.tm_mday;
                    c0 = 0;
                }
                // %H (hour 00-23)
                else if (c1 == 'H') {
                    num = time.tm_hour;
                    c0 = 0;
                }
                // %M (min 00-59)
                else if (c1 == 'M') {
                    num = time.tm_min;
                    c0 = 0;
                }
                // %z (UTC offset "+0000")
                else if (c1 == 'z') {
                    if (time.tm_isdst > 0) {
                        std::memcpy(&buff[pos], utc_offset_.daylight.data(), 5);
                    }
                    else {
                        std::memcpy(&buff[pos], utc_offset_.standard.data(), 5);
                    }
                    pos += 5;
                }
                // %y (year 25)
                else if (c1 == 'y' && time.tm_year <= (std::numeric_limits<int>::max)() - 1900) {
                    num = time.tm_year + 1900;
                    c0 = 0;
                }
                // %a (day abbreviated)
                else if (c1 == 'a' && time.tm_wday >= 0 && time.tm_wday <= 6) {
                    const auto days = "SunMonTueWedThuFriSat";
                    std::memcpy(&buff[pos], days + (time.tm_wday * 3), 3);
                    pos += 3;
                }
                // unsupported "%?"
                else {
                    buff[pos] = '%';
                    buff[pos + 1] = c1;
                    pos += 2;
                }
                // write number
                if (c0 == 0 && num >= 0) {
                    buff[pos + 1] = '0' + num % 10;
                    num /= 10;
                    buff[pos] = '0' + num % 10;
                    pos += 2;
                }
            }
            // if there is a 1 char remainder at the end of form_str
            if (form_pos < form_str_len) {
                buff[pos++] = *(time_format_.data() + form_pos++);
            }
            FSTLOG_ASSERT(pos < buff.size());
            // if not enough space in small_string<64>
            if (pos > small_string<64>::capacity()) {
                return small_string<64>{};
            }
            else {
                return small_string<64>(buff.data(), pos);
            }
        }

        /**
         * @brief Writes the seconds and fractional seconds into the formatted timestamp string.
         *
         * @details
         * - Checks if the seconds field exists in the format string (second_pos_ != 255).
         * - Converts nanoseconds to the required precision and inserts the digits into the output buffer.
         * - Uses a lookup table for efficient two-digit conversion.
         * - Handles both whole seconds and fractional seconds, inserting a decimal point if needed.
         *
         * @param nanoseconds The seconds.fraq part of the timestamp in nanoseconds.
         * @param timestring_begin Pointer to the beginning of the output timestamp string buffer.
         *
         * @note
         * - Assumes the output buffer has enough space for the seconds field.
         * - The function is called during timestamp encoding to fill in the seconds value.
         * - The seconds field position and precision are determined during initialization.
         *
         * @see prepare_seconds_placeholder
         * @see encode_timestamp
         */
        inline void write_second(
            stamp_type nanoseconds, 
            unsigned char* timestring_begin)
        {
            if (second_pos_ == 255) return; // no second in format string
            
            auto nano_ticks = nanoseconds;
            FSTLOG_ASSERT(nano_ticks < 60'000'000'000LL);

            const long long pow10[10] { 1, 10, 100, 1'000, 10'000, 100'000,
                 1'000'000, 10'000'000, 100'000'000, 1'000'000'000 };

            const int digits_cut = 9 - second_precision_;
            nano_ticks /= pow10[digits_cut];

            const char digits2[]{
                "0001020304050607080910111213141516171819"
                "2021222324252627282930313233343536373839"
                "4041424344454647484950515253545556575859"
                "6061626364656667686970717273747576777879"
                "8081828384858687888990919293949596979899" };
            const auto second_begin = timestring_begin + second_pos_;
            unsigned char* pos = second_precision_ == 0 ? 
                second_begin + 2
                : second_begin + 3 + second_precision_;
            // converting digits, two at a time
            while (nano_ticks != 0) {
                pos -= 2;
                const auto next_nano = nano_ticks / 100;
                const auto fraq = nano_ticks - next_nano * 100;
                const int digit_ind = static_cast<int>(fraq * 2);
                *pos = digits2[digit_ind];
                *(pos + 1) = digits2[digit_ind + 1];
                nano_ticks = next_nano;
            }

            // inserting decimal point
            if (second_precision_ != 0) {
                *second_begin = *(second_begin + 1);
                *(second_begin + 1) = *(second_begin + 2);
                *(second_begin + 2) = '.';
            }
        }

        time_string_cache<7> time_string_cache_;
        small_string<64> time_format_;
        detail::utc_offset utc_offset_{ detail::get_utc_offset() };
        tz_format tzone_{ tz_format::Local };
        unsigned char formatted_length_{ 0 };
        unsigned char second_precision_{ 6 };
        unsigned char second_pos_{ 255 };
    };
}
