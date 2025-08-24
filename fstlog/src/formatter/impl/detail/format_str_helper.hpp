//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string_view>

#include <detail/safe_reinterpret_cast.hpp>
#include <detail/unaligned_span.hpp>
#include <detail/utf_conv.hpp>
#include <formatter/impl/detail/logfield.hpp>
#include <formatter/impl/detail/tz_format.hpp>
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
    namespace {
        inline void uint_fromchars_4digit(
            int& number,
            const unsigned char*& begin,
            const unsigned char* end) noexcept
        {
            // there is no number
            if (begin >= end || !(*begin <= '9' && *begin >= '0')) return;

            number = *begin++ - '0';
            while (begin < end && *begin <= '9' && *begin >= '0') {
                if (number <= 999) {
                    number *= 10;
                    number += *begin - '0';
                }
                begin++;
            }
        }
                
        // write the text part of the fmt format string in to out
        inline error_code parse_fmt_text(byte_span_const& input, byte_span& output) noexcept {
            // fast path (ASCII no special {, })
            std::size_t char_num{ 0 };
            const std::size_t input_size = input.size();
            const std::size_t output_size = output.size();
            const std::size_t max_char_num = input_size < output_size ? input_size : output_size;
            while (char_num < max_char_num) {
                const auto c = input.get(char_num);
                if (c == '{' || c == '}') break; // special cases
                if (c < 0x20 || c >= 0x7F) break; // not in safe ASCII range
                char_num++;
            }
            memcpy(output.data_bytes(), input.data_bytes(), char_num);
            input.drop_front(char_num);
            output.drop_front(char_num);

            error_code error = error_code::none;
            while (!input.empty()) {
                // special cases double, begin, end
                if (input.template get<0>() == '{' || input.template get<0>() == '}') {
                    // double "{{" or "}}" written as single "{" or "}"
                    if (input.size() > 1 && input.template get<0>() == input.template get<1>()) {
                        if (output.empty()) {
                            error = error_code::buff_full;
                            break;
                        }
                        output.template set<0>(input.template get<0>());
                        input.template drop_front<2>();
                        output.template drop_front<1>();
                    }
                    // "}" error end marker not expected
                    else if (input.template get<0>() == '}') {
                        error = error_code::fmt_bad;
                        break;
                    }
                    // "{" end of text
                    else {
                        break;
                    }
                }
                else {
                    if (output.size() < 10) {
                        error = error_code::buff_full;
                        break;
                    }
                    const auto codepoint = detail::utf::decode_utf8_char(input);
                    detail::utf::encode_safe_utf8_char(codepoint, output);
                }
            }
            return error;
        }

        // parse the replacement field part of the fmt format string in to out
        inline error_code parse_fmt_repl_field(
            byte_span_const& input,
            byte_span_const& field_name,
            byte_span_const& format_spec) noexcept
        {
            FSTLOG_ASSERT(
                input.data_bytes() != nullptr 
                && !input.empty()
                && input.template get<0>() == '{');
            input.template drop_front<1>();
            bool name_set = false;
            const auto name_begin = input.data_bytes();
            auto name_end = name_begin;
            auto spec_begin = name_begin;
            auto spec_end = spec_begin;
            error_code error = error_code::fmt_bad;
            while (!input.empty()) {
                if (input.template get<0>() == '}') {
                    if (!name_set) {
                        name_end = input.data_bytes();
                        spec_begin = name_end;
                    }
                    spec_end = input.data_bytes();
                    input.template drop_front<1>();
                    error = error_code::none;
                    break;
                }
                else if (input.template get<0>() == '{') {
                    // error unmatched '{'
                    break;
                }
                else if (!name_set && input.template get<0>() == ':') {
                    name_end = input.data_bytes();
                    spec_begin = input.data_bytes() + 1;
                    name_set = true;
                }
                input.template drop_front<1>();
            }

            if (error != error_code::none) {
                name_end = name_begin;
                spec_begin = spec_end;
            }
            field_name = byte_span_const(name_begin, static_cast<std::size_t>(name_end - name_begin));
            format_spec = byte_span_const(spec_begin, static_cast<std::size_t>(spec_end - spec_begin));
            return error;
        }

        inline const unsigned char* skip_fill_align(
            const unsigned char* begin,
            const unsigned char* end) noexcept
        {
            if (begin >= end) return begin;
            auto pos = begin;
            unaligned_span<const unsigned char> input(pos, static_cast<std::size_t>(end - pos));
            std::uint32_t first_char = detail::utf::decode_utf8_char(input);
            pos = input.data_bytes();
            // check if there is a good fill char (first_char) + align char
            if (pos < end) {
                const unsigned char align_char{ *pos };
                if (align_char == '<' ||
                    align_char == '>' ||
                    align_char == '^')
                {
                    if (first_char != '{' && first_char != '}'
                        && detail::utf::safe_utf_code_point(first_char))
                    {
                        // skip align char (and fill char)
                        return pos + 1;
                    }
                    else {
                        // do not skip invalid/unsafe fill char (+ align char)
                        return begin;
                    }
                }
            }
            // if there was no fill char + align char
            // check if there is a single align char
            if (first_char == '<' ||
                first_char == '>' ||
                first_char == '^')
            {
                // skip single align char
                return pos;
            }
            // nothing to skip
            return begin;
        }

        inline const unsigned char* skip_sign_alt_0(
            const unsigned char* begin,
            const unsigned char* end) noexcept
        {
            auto out = begin;
            if (out < end
                && ((*out == '+') | (*out == '-') | (*out == ' ')) == 1) 
            {
                out++;
            }
            if (out < end && *out == '#') out++;
            if (out < end && *out == '0') out++;
            return out;
        }

        inline const unsigned char* skip_numbers(
            const unsigned char* begin,
            const unsigned char* end) noexcept
        {
            auto out = begin;
            while (out != end && *out >= '0' && *out <= '9') {
                out++;
            }
            return out;
        }

        inline tz_format get_zone(byte_span_const &format_str) noexcept {
            tz_format out{ tz_format::Local };
            if (!format_str.empty()) {
                if (format_str.template get<0>() == 'L') {
                    format_str.template drop_front<1>();
                }
                else if (format_str.template get<0>() == 'U') {
                    out = tz_format::UTC;
                    format_str.template drop_front<1>();
                }
            }
            return out;
        }

        inline int get_width(
            const unsigned char*& begin,
            const unsigned char* end) noexcept
        {
            int width{ 0 };
            uint_fromchars_4digit(width, begin, end);
            return width;
        }

        inline void get_precision(
            int &precision,
            const unsigned char*& begin,
            const unsigned char* end) noexcept
        {
            if (begin < end && *begin == '.') {
                uint_fromchars_4digit(precision, ++begin, end);
            }
        }

        inline error_code time_format(
            byte_span_const& form_spec,
            byte_span_const& time_fmt) noexcept
        {
            auto error = error_code::none;
            if (form_spec.empty()) {
                time_fmt = form_spec;
                return error;
            }
            const auto begin = form_spec.data_bytes();
            const auto end = begin + form_spec.size_bytes();
            auto pos = begin;
            pos = skip_fill_align(pos, end);
            const auto pos_0{ pos };
            pos = skip_sign_alt_0(pos, end);
            if (pos != pos_0) {
                // "[sign][#][0] not supported in timestamp formatting (format string)!"
                error = error_code::fmt_bad;
            }
            pos = skip_numbers(pos, end);
            form_spec = byte_span_const{ begin, static_cast<std::size_t>(pos - begin) };
            time_fmt = byte_span_const{ pos, static_cast<std::size_t>(end - pos) };
            return error;
        }

        inline logfield get_repl_field_id(byte_span_const name) noexcept {
            auto str_v = std::string_view{
                safe_reinterpret_cast<const char*>(name.data_bytes()), name.size_bytes() };
            if (str_v == "time") return logfield::Timestamp;
            if (str_v == "message") return logfield::Message;
            if (str_v == "level") return logfield::Severity;
            if (str_v == "file") return logfield::File;
            if (str_v == "line") return logfield::Line;
            if (str_v == "function") return logfield::Function;
            if (str_v == "thread") return logfield::Thread;
            if (str_v == "logger") return logfield::Logger;
            if (str_v == "channel") return logfield::Channel;
            if (str_v == "policy") return logfield::Policy;
            if (str_v == "severity") return logfield::Severity;
            if (str_v == "timestamp") return logfield::Timestamp;
            return logfield::Invalid;
        }

        inline std::string_view get_repl_field_name(logfield field_id) noexcept {
            switch (field_id) {
            case logfield::Severity: return "severity";
            case logfield::Policy: return "policy";
            case logfield::Channel:    return "channel";
            case logfield::Timestamp: return "timestamp";
            case logfield::Thread: return "thread";
            case logfield::Logger: return "logger";
            case logfield::File: return "file";
            case logfield::Line: return "line";
            case logfield::Function: return "function";
            case logfield::Message:    return "message";
            default: return "invalid";
            }
        }

        inline bool valid_fmt_type_spec(unsigned char type_spec) noexcept {
            // LUT for chars 'A' - 'x' range in a single 64 bit uint
            constexpr std::uint64_t valid_type_spec_lut =
                  (1ULL << ('A' - 'A')) | (1ULL << ('a' - 'A'))
                | (1ULL << ('B' - 'A')) | (1ULL << ('b' - 'A'))
                | (1ULL << ('c' - 'A')) | (1ULL << ('d' - 'A'))
                | (1ULL << ('E' - 'A')) | (1ULL << ('e' - 'A'))
                | (1ULL << ('F' - 'A')) | (1ULL << ('f' - 'A'))
                | (1ULL << ('G' - 'A')) | (1ULL << ('g' - 'A'))
                | (1ULL << ('o' - 'A')) | (1ULL << ('p' - 'A'))
                | (1ULL << ('s' - 'A'))
                | (1ULL << ('x' - 'A')) | (1ULL << ('X' - 'A'));
            // bounds check
            if (type_spec < 'A' || type_spec > 'x') return false;
            
            // calculate the bit position
            const auto bit_pos = type_spec - 'A';

            // shift the LUT bit to the least significant place and check if it is set.
            return (valid_type_spec_lut >> bit_pos) & 1;
        }

        // if a number begins at pos 'begin' skips the digits
        // and checks if it is valid
        inline bool skip_valid_fmt_number(
            const unsigned char* &begin,
            const unsigned char* end) noexcept
        {
            FSTLOG_ASSERT(begin <= end);
            constexpr int max_digits = 4;
            // no number to skip
            if (begin == end || *begin < '0' || *begin > '9') return true;
            // valid number can not begin with '0'
            if (*begin++ == '0') return false;
            int digits = 1;
            while (digits <= max_digits && begin < end
                && *begin >= '0' && *begin <= '9') 
            {
                begin++;
                digits++;
            }
            return (digits <= max_digits);
        }

        inline bool valid_format_spec(byte_span_const format_spec) {
            if (format_spec.empty()) return true;
            auto pos = format_spec.data_bytes();
            const auto end = pos + format_spec.size_bytes();
            
            // skip the fill_char AND the alignment specifier if valid + safe
            pos = skip_fill_align(pos, end);
            // if not the following checks will fail 
                        
            // sign '#' '0'
            pos = skip_sign_alt_0(pos, end);
            // width
            if(!skip_valid_fmt_number(pos, end)) return false;
            if (pos == end) return true;
            // if there is a precision specifier
            if (*pos == '.') {
                pos++; // skip dot
                const auto prec_begin = pos;
                if (!skip_valid_fmt_number(pos, end)) return false;
                // number is mandatory after a '.'
                if (prec_begin == pos) return false;
            }
            if (pos == end) return true;
            // 'L' use local decimal separator specifier
            if (*pos == 'L') pos++;
            // type is not mandatory
            if (pos == end) return true;
            // type must be 1 char
            if (end - pos != 1) return false;
            return valid_fmt_type_spec(*pos);
        }
    }
}
