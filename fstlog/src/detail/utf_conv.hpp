//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

#include <detail/unaligned_span.hpp>
#include <detail/utf8_len.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/error_code.hpp>

namespace fstlog::detail::utf {
    namespace constants {
        inline constexpr std::uint32_t replacement_char = 0xFFFD;
        inline constexpr std::uint32_t max_code_p = 0x10FFFF;
        inline constexpr std::uint32_t surrogate_start = 0xD800;
        inline constexpr std::uint32_t max_high_surrogate = 0xDBFF;
        inline constexpr std::uint32_t surrogate_end = 0xDFFF;
        inline constexpr std::uint32_t min_2_byte_code_p = 0x80;
        inline constexpr std::uint32_t min_3_byte_code_p = 0x800;
        inline constexpr std::uint32_t min_4_byte_code_p = 0x10000;

        // Masks and lead patterns for continuation bytes
        inline constexpr std::uint32_t cont_prefix_mask = 0b1100'0000;
        inline constexpr std::uint32_t cont_prefix = 0b1000'0000;
        inline constexpr std::uint32_t cont_data_mask = 0b0011'1111;
    }

    struct conversion_result {
        std::size_t codepoints_written{ 0 };
        error_code ec;
        
        // codepoints_written contains the number of the codepoints 
        // written to the output this can be larger than 
        // the codepoints read from the input do to escaping 
        // (1 unsafe codepoint -> 6/10 codepoints)
    };

    // validates data before casting to char8/16/32_t (no op if same unsigned 8/16/32 bit)
    template <std::size_t bitnum, typename T>
    inline constexpr bool valid_utf_char(T utf_char) noexcept {
        static_assert(bitnum == 8 || bitnum == 16 || bitnum == 32, "Invalid bitnum!");
        static_assert(std::is_integral_v<T>, "Invalid type!");
        // valid utf_char: 0 - max_val
        constexpr std::uint64_t max_val = (std::uint64_t(1) << bitnum) - 1;

        if constexpr ((std::numeric_limits<T>::min)() < 0) {
            if (utf_char < 0) return false;
        }
        if constexpr ((std::numeric_limits<T>::max)() > max_val) {
            if (utf_char > max_val) return false;
        }
        return true;
    }
    
    // Semantic check for surrogates, and out-of-range values.
    constexpr bool valid_utf_code_point(std::uint32_t code_point) noexcept {
        return ( code_point < utf::constants::surrogate_start  // under surrogate range
                || (code_point > utf::constants::surrogate_end // between surrogate end and max
                    && code_point <= utf::constants::max_code_p));
    }

    // Determines if a valid utf code point is in the safe whitelist,
    // exludes invalid out of range and surrogate values.
    constexpr bool safe_utf_code_point(std::uint32_t code_point) noexcept {
        // The array contains range boundaries between
        // unsafe/safe ranges at even indexes [0,2,4...] (first safe codepoint)
        // and safe/unsafe ranges at odd indexes [1,3,5...] (first unsafe codepoint).
        // Values have to be in increasing order.
        alignas(fstlog::constants::cache_ls_nosharing) constexpr std::array<std::uint16_t, 18> sparse_lut{
            0x0020, 0x007F, // Printable ASCII
            0x00A1, 0x02B0, // Latin-1 Supplement, extended, IPA
            0x0370, 0x0700, // Greek, Cyrillic, Armenian, Hebrew, Arabic
            0x0900, 0x0980, // Devanagari
            0x20A0, 0x20C0, // Currency
            0x3001, 0x303E, // Hiragana, Katakana
            0x3041, 0x3100, // Hiragana, Katakana
            0x3300, 0xA4C7, // CJK, YI
            0xAC00, 0xD7A4  // Hangul Syllables
        };
        
        // Why not binary search? 
        // 1. Short array makes linear search faster (data in one cache line).
        // 2. Majority of characters used in logging is ASCII and western language (early exit). 

        bool is_safe = false; // we start with an unsafe range [0, 0x0020) 
        for (auto boundary : sparse_lut) {
            if (code_point < boundary) return is_safe; // we found the range that the code_point is in
            is_safe = !is_safe; // boundary marks always a change, we flip is_safe when stepping over 
        }

        // Array with the same structure as above but for larger 4 byte codepoints.
        alignas(fstlog::constants::cache_ls_nosharing) constexpr std::array<std::uint32_t, 4> sparse_lut_b{
            0x1F300, 0x1F3FB, // Miscellaneous symbols
            0x1F400, 0x1F6D8  // Miscellaneous symbols, Emoticons
        };

        for (auto boundary : sparse_lut_b) {
            if (code_point < boundary) return is_safe;
            is_safe = !is_safe;
        }

        return false;
    }

    static_assert(!safe_utf_code_point(utf::constants::replacement_char), "Replacement char must be unsafe!");

    // encodes the code_point escaped \uXXXX or \UYYYYYYYY
    // returns the length of the escape sequence
    // precondition: output.size() >= 10
    template<typename T>
    inline std::size_t encode_escaped(
        std::uint32_t code_point,
        unaligned_span<T> &output) noexcept
    {
        static_assert(std::is_integral_v<T> && (std::numeric_limits<T>::max)() >= 127,
            "Invalid type!");
        FSTLOG_ASSERT(output.data_bytes() != nullptr);
        FSTLOG_ASSERT(code_point <= utf::constants::max_code_p);
        FSTLOG_ASSERT(output.size() >= 10);
        std::size_t str_length = code_point <= 0xffff ? 6 : 10;
        
        constexpr std::array<unsigned char, 16> hex_digits{
            '0', '1', '2', '3', '4', '5', '6', '7',
            '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

        if (str_length == 6) {
            output.template set<0>('\\');
            output.template set<1>('u');
            output.template set<2>(hex_digits[code_point >> 12]);
            output.template set<3>(hex_digits[(code_point >> 8) & 0x0F]);
            output.template set<4>(hex_digits[(code_point >> 4) & 0x0F]);
            output.template set<5>(hex_digits[code_point & 0x0F]);
            output.template drop_front<6>();
        }
        else {
            output.template set<0>('\\');
            output.template set<1>('U');
            output.template set<2>('0');
            output.template set<3>('0');
            output.template set<4>(hex_digits[(code_point >> 20) & 0x0F]); // must mask for bad code point in release build  
            output.template set<5>(hex_digits[(code_point >> 16) & 0x0F]);
            output.template set<6>(hex_digits[(code_point >> 12) & 0x0F]);
            output.template set<7>(hex_digits[(code_point >> 8) & 0x0F]);
            output.template set<8>(hex_digits[(code_point >> 4) & 0x0F]);
            output.template set<9>(hex_digits[code_point & 0x0F]);
            output.template drop_front<10>();
        }
        return str_length;
    }

    // Decodes a single UTF-8 code point from the buffer.
    // Advances input by the number of T-s consumed.
    // Returns 0xFFFD on decoding error, consuming the minimal invalid subsequence
    // Precondition: !input.empty().
    template<typename T>
    [[nodiscard]] constexpr std::uint32_t decode_utf8_char(
        unaligned_span<const T> &input) noexcept
    {
        static_assert(std::is_integral_v<T>, "Invalid type!");
        FSTLOG_ASSERT(input.data_bytes() != nullptr && !input.empty());
        
        // Load the first T.
        const T first_b = input.template get<0>();
        if (!valid_utf_char<8>(first_b)) {
            input.template drop_front<1>();
            return utf::constants::replacement_char;
        }
        const std::uint32_t b1 = static_cast<std::uint32_t>(first_b);

        // --- Case 1: ASCII (1 byte: 0xxxxxxx) ---
        // Most common case in Western languages and logs.
        if (b1 < 0x80) {
            input.template drop_front<1>();
            return b1;
        }

        // --- Case 2: 2 Bytes (110xxxxx 10xxxxxx) ---
        // checking 2 byte sequence lead bits
        if ((b1 & 0b1110'0000) == 0b1100'0000) {
            // Structural Check 1: missing 2. byte?
            if (input.size() < 2) {
                input.template drop_front<1>(); // Not enough data, consume lead T only.
                return utf::constants::replacement_char;
            }

            // Load the second T.
            const T second_b = input.template get<1>();
            if (!valid_utf_char<8>(second_b)) {
                input.template drop_front<2>(); // second T was invalid, consume all 2.
                return utf::constants::replacement_char;
            }
            const std::uint32_t b2 = static_cast<std::uint32_t>(second_b);

            // Structural Check 2: Continuation byte validity
            if ((b2 & utf::constants::cont_prefix_mask) != utf::constants::cont_prefix) {
                input.template drop_front<1>(); // Invalid continuation byte, consume first only.
                return utf::constants::replacement_char;
            }

            // Decode
            std::uint32_t cp = ((b1 & 0b0001'1111) << 6) 
                | (b2 & utf::constants::cont_data_mask);

            // Semantic Check: Overlong encoding (must be >= U+0080)
            if (cp < utf::constants::min_2_byte_code_p) {
                input.template drop_front<2>(); // Structurally valid but semantically wrong, consume all 2.
                return utf::constants::replacement_char;
            }

            // Success
            input.template drop_front<2>();
            return cp;
        }

        // --- Case 3: 3 Bytes (1110xxxx 10xxxxxx 10xxxxxx) ---
        // checking 3 byte sequence lead bits
        else if ((b1 & 0b1111'0000) == 0b1110'0000) {
            // Structural Check 1: missing continuation bytes?
            if (input.size() < 3) {
                input.template drop_front<1>(); // Not enough data, consume lead only.
                return utf::constants::replacement_char;
            }

            // Load the second T.
            const T second_b = input.template get<1>();
            if (!valid_utf_char<8>(second_b)) {
                input.template drop_front<2>(); // second T was invalid, consume 2.
                return utf::constants::replacement_char;
            }
            const std::uint32_t b2 = static_cast<std::uint32_t>(second_b);
            // Structural Check 2: Continuation byte 1 (Minimal consumption rule)
            if ((b2 & utf::constants::cont_prefix_mask) != utf::constants::cont_prefix) {
                input.template drop_front<1>(); // b2 non continuation byte, consume b1
                return utf::constants::replacement_char;
            }

            // Load the third T.
            const T third_b = input.template get<2>();
            if (!valid_utf_char<8>(third_b)) {
                input.template drop_front<3>(); // third T was invalid, consume all 3.
                return utf::constants::replacement_char;
            }
            const std::uint32_t b3 = static_cast<std::uint32_t>(third_b);
            // Structural Check 3: Continuation byte 2 (Minimal consumption rule)
            if ((b3 & utf::constants::cont_prefix_mask) != utf::constants::cont_prefix) {
                input.template drop_front<2>(); // b3 non continuation byte, consume b1 and valid b2
                return utf::constants::replacement_char;
            }

            // Decode
            std::uint32_t cp = ((b1 & 0b0000'1111) << 12)
                | ((b2 & utf::constants::cont_data_mask) << 6)
                | (b3 & utf::constants::cont_data_mask);

            // Semantic Checks:
            // 1. Overlong encoding (must be >= U+0800)
            // 2. Surrogates (U+D800 to U+DFFF are invalid)
            if (cp < utf::constants::min_3_byte_code_p 
                || (cp >= utf::constants::surrogate_start && cp <= utf::constants::surrogate_end))
            {
                input.template drop_front<3>(); // Structurally valid but semantically wrong, consume all.
                return utf::constants::replacement_char;
            }

            // Success
            input.template drop_front<3>();
            return cp;
        }

        // --- Case 4: 4 Bytes (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx) ---
        // checking 4 byte sequence lead bits
        else if ((b1 & 0b1111'1000) == 0b1111'0000) {
            // Structural Check 1: missing continuation bytes?
            if (input.size() < 4) {
                input.template drop_front<1>(); // Not enough data, consume lead only
                return utf::constants::replacement_char;
            }

            // Load the second T.
            const T second_b = input.template get<1>();
            if (!valid_utf_char<8>(second_b)) {
                input.template drop_front<2>(); // second T was invalid, consume 2.
                return utf::constants::replacement_char;
            }
            const std::uint32_t b2 = static_cast<std::uint32_t>(second_b);
            // Structural Check 2: Continuation byte 1 (Minimal consumption rule)
            if ((b2 & utf::constants::cont_prefix_mask) != utf::constants::cont_prefix) {
                input.template drop_front<1>(); // b2 non continuation byte, consume b1
                return utf::constants::replacement_char;
            }

            // Load the third T.
            const T third_b = input.template get<2>();
            if (!valid_utf_char<8>(third_b)) {
                input.template drop_front<3>(); // third T was invalid, consume 3.
                return utf::constants::replacement_char;
            }
            const std::uint32_t b3 = static_cast<std::uint32_t>(third_b);
            // Structural Check 3: Continuation byte 2 (Minimal consumption rule)
            if ((b3 & utf::constants::cont_prefix_mask) != utf::constants::cont_prefix) {
                input.template drop_front<2>(); // b3 non continuation byte, consume b1, b2
                return utf::constants::replacement_char;
            }

            // Load the fourth T.
            const T fourth_b = input.template get<3>();
            if (!valid_utf_char<8>(fourth_b)) {
                input.template drop_front<4>(); // fourth T was invalid, consume all 4.
                return utf::constants::replacement_char;
            }
            const std::uint32_t b4 = static_cast<std::uint32_t>(fourth_b);
            // Structural Check 4: Continuation byte 3 (Minimal consumption rule)
            if ((b4 & utf::constants::cont_prefix_mask) != utf::constants::cont_prefix) {
                input.template drop_front<3>(); // b4 non continuation byte, consume b1, b2, b3
                return utf::constants::replacement_char;
            }

            // Decode
            std::uint32_t cp = ((b1 & 0b0000'0111) << 18)
                | ((b2 & utf::constants::cont_data_mask) << 12)
                | ((b3 & utf::constants::cont_data_mask) << 6)
                | (b4 & utf::constants::cont_data_mask);

            // Semantic Checks:
            // 1. Overlong encoding (must be >= U+10000)
            // 2. Exceeding Unicode maximum (must be <= U+10FFFF)
            if (cp < utf::constants::min_4_byte_code_p || cp > utf::constants::max_code_p) {
                input.template drop_front<4>(); // Structurally valid but semantically wrong, consume all.
                return utf::constants::replacement_char;
            }

            // Success
            input.template drop_front<4>();
            return cp;
        }

        // --- Case 5: Error (Invalid lead byte) ---
        // Handles bytes starting with 10xxxxxx (which are continuation bytes)
        // or 11111xxx (invalid sequences).
        else {
            input.template drop_front<1>(); // Consume the single invalid T.
            return utf::constants::replacement_char;
        }
    }

    // Encodes the code_point 
    // into an utf-8 sequence if it is considered safe
    // or into an ASCII escape string if unsafe.
    // Returns the codepoints written to the buffer (char counter for post processing)
    // Advances the output.
    // Precondition: code_point is a valid UTF code point.
    // Precondition: output.size() >= 10 (space for a 10 char length escape sequence)
    template<typename T>
    inline std::size_t encode_safe_utf8_char(
        std::uint32_t code_point,
        unaligned_span<T> &output) noexcept
    {
        static_assert(std::is_integral_v<T> 
            && (std::numeric_limits<T>::max)() >= 255, "Invalid type!");
        FSTLOG_ASSERT(output.data_bytes() != nullptr);
        FSTLOG_ASSERT(output.size() >= 10);
        FSTLOG_ASSERT(valid_utf_code_point(code_point));
        
        if (!safe_utf_code_point(code_point)) {
            return encode_escaped(code_point, output);
        }

        // 1 byte sequence (0xxxxxxx)
        if (code_point < 0x80) {
            output.template set<0>(static_cast<T>(code_point));
            output.template drop_front<1>();
        }
        // 2 byte sequence (110xxxxx 10xxxxxx)
        else if (code_point < utf::constants::min_3_byte_code_p) {
            output.template set<0>(static_cast<T>(
                (code_point >> 6) | std::uint32_t{ 0b1100'0000 }));
            output.template set<1>(static_cast<T>(
                (code_point & utf::constants::cont_data_mask) | utf::constants::cont_prefix));
            output.template drop_front<2>();
        }
        // 3 byte sequence (1110xxxx 10xxxxxx 10xxxxxx)
        else if (code_point < utf::constants::min_4_byte_code_p) {
            output.template set<0>(static_cast<T>(
                (code_point >> 12) | std::uint32_t{ 0b1110'0000 }));
            output.template set<1>(static_cast<T>(
                ((code_point >> 6) & utf::constants::cont_data_mask) | utf::constants::cont_prefix));
            output.template set<2>(static_cast<T>(
                (code_point & utf::constants::cont_data_mask) | utf::constants::cont_prefix));
            output.template drop_front<3>();
        }
        // 4 byte sequence (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
        else {
            output.template set<0>(static_cast<T>(
                (code_point >> 18) | std::uint32_t{ 0b1111'0000 }));
            output.template set<1>(static_cast<T>(
                ((code_point >> 12) & utf::constants::cont_data_mask) | utf::constants::cont_prefix));
            output.template set<2>(static_cast<T>(
                ((code_point >> 6) & utf::constants::cont_data_mask) | utf::constants::cont_prefix));
            output.template set<3>(static_cast<T>(
                (code_point & utf::constants::cont_data_mask) | utf::constants::cont_prefix));
            output.template drop_front<4>();
        }
        return 1;
    }
    
    template <typename T>
    [[nodiscard]] constexpr std::uint32_t decode_utf16_char(
        unaligned_span<const T> &input) noexcept
    {
        static_assert(std::is_integral_v<T>, "Invalid type!");
        FSTLOG_ASSERT(input.data_bytes() != nullptr && !input.empty());
        T utf16_char{ input.template get<0>() };
        if (!valid_utf_char<16>(utf16_char)) {
            input.template drop_front<1>();
            return utf::constants::replacement_char;
        }

        // non surrogate range
        if (utf16_char < constants::surrogate_start 
            || utf16_char > constants::surrogate_end)
        {
            input.template drop_front<1>();
            return static_cast<std::uint32_t>(utf16_char);
        }

        // surrogate range
            
        // missing surrogate
        if (input.size() < 2) {
            input.template drop_front<1>();
            return utf::constants::replacement_char;
        }
        T utf16_char_surr_low{ input.template get<1>()}; // validity check is not needed see range check below.

        // if surrogtes are valid
        if (utf16_char <= constants::max_high_surrogate 
            && utf16_char_surr_low > constants::max_high_surrogate 
            && utf16_char_surr_low <= constants::surrogate_end)
        {
            const std::uint32_t surrogate_high = static_cast<std::uint32_t>(utf16_char);
            const std::uint32_t surrogate_low = static_cast<std::uint32_t>(utf16_char_surr_low);
            std::uint32_t code_p =
                ((surrogate_low & 0x3FF) | ((surrogate_high & 0x3FF) << 10)) + 0x10000;
            input.template drop_front<2>();
            return code_p;
        }
                    
        // invalid surrogates or second is non surrogate (can be valid)
        input.template drop_front<1>();
        return utf::constants::replacement_char;
    }

    template <typename I, typename O>
    inline conversion_result utf32_to_utf8(
        unaligned_span<const I> &input,
        unaligned_span<O> &output,
        std::size_t max_input_codepoints = (std::numeric_limits<std::size_t>::max)()) noexcept
    {
        static_assert(std::is_integral_v<I>, "Invalid type!");
        FSTLOG_ASSERT(input.data_bytes() != nullptr);
        FSTLOG_ASSERT(output.data_bytes() != nullptr);
        conversion_result result{0, error_code::none};
        std::size_t codepoints_consumed{ 0 };
        while (!input.empty() && codepoints_consumed < max_input_codepoints) {
            if (output.size() < 10) {
                result.ec = error_code::buff_full;
                break;
            }
            I char_utf32 = input.template get<0>();
            input.template drop_front<1>();
            std::uint32_t code_point = static_cast<std::uint32_t>(char_utf32);
            if (!valid_utf_char<32>(char_utf32)) code_point = utf::constants::replacement_char;
            if (!valid_utf_code_point(code_point)) code_point = utf::constants::replacement_char;
            const auto out_count = encode_safe_utf8_char(code_point, output);
            result.codepoints_written += out_count;
            codepoints_consumed++;
        }
        return result;
    }

    template <typename I, typename O>
    inline conversion_result utf16_to_utf8(
        unaligned_span<const I> &input,
        unaligned_span<O> &output,
        std::size_t max_input_codepoints = (std::numeric_limits<std::size_t>::max)()) noexcept
    {
        FSTLOG_ASSERT(input.data_bytes() != nullptr);
        FSTLOG_ASSERT(output.data_bytes() != nullptr);
        conversion_result result{ 0, error_code::none };
        std::size_t codepoints_consumed{ 0 };
        while (!input.empty() && codepoints_consumed < max_input_codepoints) {
            if (output.size() < 10) {
                result.ec = error_code::buff_full;
                break;
            }
            std::uint32_t code_p = decode_utf16_char(input);
            const auto out_count = encode_safe_utf8_char(code_p, output);
            result.codepoints_written += out_count;
            codepoints_consumed++;
        }
        return result;
    }

    template<typename I, typename O>
    inline conversion_result utf8_to_utf8(
        unaligned_span<const I>& input,
        unaligned_span<O>& output,
        std::size_t max_input_codepoints = (std::numeric_limits<std::size_t>::max)()) noexcept
    {
        static_assert(
            std::is_integral_v<O>
            && (std::numeric_limits<O>::max)() >= 255,
            "Invalid type!");
        FSTLOG_ASSERT(input.data_bytes() != nullptr);
        FSTLOG_ASSERT(output.data_bytes() != nullptr);
        // fast path safe ASCII
        std::size_t max_ascii = input.size();
        if (max_ascii > max_input_codepoints) max_ascii = max_input_codepoints;
        const std::size_t dest_count = output.size();
        if (max_ascii > dest_count) max_ascii = dest_count;

        std::size_t char_count = 0;
        while (char_count < max_ascii){
            const auto code = input.get(char_count);
            if (code < 0x20 || code >= 0x7F) break; // non valid ASCII
            char_count++;
        }
        if constexpr (std::is_same_v<std::remove_cv_t<I>, O>) {
            std::memcpy(output.data_bytes(), input.data_bytes(), char_count * sizeof(I));
        }
        else {
            std::size_t ind = 0;
            while (ind < char_count) {
                output.set(ind, static_cast<O>(input.get(ind)));
                ind++;
            }
        }
        
        input.drop_front(char_count);
        output.drop_front(char_count);

        conversion_result result{ char_count, error_code::none };
        std::size_t codepoints_consumed{ char_count };
        while (!input.empty() && codepoints_consumed < max_input_codepoints) {
            if (output.size() < 10) {
                result.ec = error_code::buff_full;
                break;
            }
            auto temp = input;
            std::uint32_t code_point = decode_utf8_char(input);
            if (safe_utf_code_point(code_point)) {
                // 1 byte sequence (0xxxxxxx)
                if (code_point < utf::constants::min_2_byte_code_p) {
                    output.template set<0>(static_cast<O>(code_point));
                    output.template drop_front<1>();
                }
                // 2 byte sequence (110xxxxx 10xxxxxx)
                else if (code_point < utf::constants::min_3_byte_code_p) {
                    output.template set<0>(static_cast<O>(temp.template get<0>()));
                    output.template set<1>(static_cast<O>(temp.template get<1>()));
                    output.template drop_front<2>();
                }
                // 3 byte sequence (1110xxxx 10xxxxxx 10xxxxxx)
                else if (code_point < utf::constants::min_4_byte_code_p) {
                    output.template set<0>(static_cast<O>(temp.template get<0>()));
                    output.template set<1>(static_cast<O>(temp.template get<1>()));
                    output.template set<2>(static_cast<O>(temp.template get<2>()));
                    output.template drop_front<3>();
                }
                // 4 byte sequence (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
                else {
                    output.template set<0>(static_cast<O>(temp.template get<0>()));
                    output.template set<1>(static_cast<O>(temp.template get<1>()));
                    output.template set<2>(static_cast<O>(temp.template get<2>()));
                    output.template set<3>(static_cast<O>(temp.template get<3>()));
                    output.template drop_front<4>();
                }
                result.codepoints_written += 1;
            }
            else {
                auto char_num = encode_escaped(code_point, output);
                result.codepoints_written += char_num;
            }
            codepoints_consumed++;
        }
        return result;
    }
    
    // if error is buff_full, the output contains all 
    // codepoints consumed from the input
    // if the error is input_bad, the last consumed 
    // (bad) codepoint from the input is not written to the output
    template<typename I, typename O>
    inline conversion_result safe_utf8_to_utf16(
        unaligned_span<const I>& input,
        unaligned_span<O>& output) noexcept
    {
        FSTLOG_ASSERT(input.data_bytes() != nullptr);
        conversion_result result{ 0, error_code::none };
        while (!input.empty()) {
            if (output.empty()) {
                result.ec = error_code::buff_full;
                break;
            }
            std::uint32_t code_point = decode_utf8_char(input);
            // only safe non surrogate pair codepoints are allowed
            if (code_point > 0xFFFF || !safe_utf_code_point(code_point)) {
                result.ec = error_code::input_bad;
                // we exit here consuming but not writing the "bad" code_point
                break;
            }
            // write the codepoint (non surrogate pair)
            output.template set<0>(static_cast<O>(code_point));
            output.template drop_front<1>();
            result.codepoints_written++;
        }
        return result;
    }

    template<int utf_bits, typename I, typename O>
    inline conversion_result utf8conv(
        unaligned_span<const I>& input,
        unaligned_span<O>& output,
        std::size_t max_input_codepoints = (std::numeric_limits<std::size_t>::max)()) noexcept
    {
        if constexpr (utf_bits == 32) {
            return utf32_to_utf8(input, output, max_input_codepoints);
        }
        else if constexpr (utf_bits == 16) {
            return utf16_to_utf8(input, output, max_input_codepoints);
        }
        else if constexpr (utf_bits == 8) {
            return utf8_to_utf8(input, output, max_input_codepoints);
        }
        else {
            static_assert(!sizeof(I), "Invalid bit number!");
            return conversion_result{};
        }
    }

    /*
    * @brief Calculates the length of a UTF-8 string (byte and char) trimmed to the desired char length.
    * If the string is shorter than the trim length, the full lengths are returned.
    *
    * @param input defining the input UTF-8 buffer.
    * @param trim_length The desired trimmed length (in utf codepoints). 
    * @return {byte_count, char_count} where:
    *         - byte_count: byte length of the trimmed string
    *         - char_count: character length of the trimmed string
    */
    template<typename T>
    inline utf8_len utf8_str_trim(
        unaligned_span<const T> input,
        std::size_t trim_length = (std::numeric_limits<std::size_t>::max)()) noexcept 
    {
        FSTLOG_ASSERT(input.data_bytes() != nullptr);
        const auto input_size = input.size();
        std::size_t max_ascii_len = input_size > trim_length ? trim_length : input_size;
        std::size_t char_num{ 0 };
        while (char_num < max_ascii_len
            && input.get(char_num) >= 0x00 && input.get(char_num) < 0x80) char_num++;
        input.drop_front(char_num);
        while (char_num < trim_length && !input.empty()) {
            [[maybe_unused]] auto code_point = utf::decode_utf8_char(input);
            char_num++;
        }
        return { input_size - input.size(), char_num };
    }
}
