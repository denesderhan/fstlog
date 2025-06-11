//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>

#include <detail/byte_span.hpp>
#include <detail/utf8_helper.hpp>
#include <formatter/impl/detail/logfield.hpp>
#include <formatter/impl/detail/tz_format.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
	static_assert(
		'0' + 1 == '1' && '0' + 2 == '2' && '0' + 3 == '3'
		&& '0' + 4 == '4' && '0' + 5 == '5' && '0' + 6 == '6'
		&& '0' + 7 == '7' && '0' + 8 == '8' && '0' + 9 == '9');

	inline void uint_fromchars_4digit(
		int& number,
		const unsigned char*& begin,
		const unsigned char* end) noexcept
	{
		// there is no number
		if (begin >= end || !(*begin <= '9' && *begin >= '0')) return;

		number = *begin++ - '0';
		while (begin < end && *begin <= '9' && *begin >= '0') {
			if (number < 9999) {
				number *= 10;
				number += *begin - '0';
			}
			begin++;
		}
		if (number > 9999) number = 9999;
	}

	// write the text part of the fmt format string in to out
	inline error_code parse_fmt_text(
		unsigned char const*& in_pos, unsigned char const* in_end,
		unsigned char*& out_pos, unsigned char const* out_end) noexcept
	{
		FSTLOG_ASSERT(in_pos != nullptr && out_pos != nullptr);
		error_code error = error_code::none;
		while (in_pos < in_end) {
			bool skip = false;
			if (*in_pos == '{' || *in_pos == '}') {
				if (in_end - in_pos > 1 && *(in_pos + 1) == *in_pos) {
					// skip the duplicated '{' or '}'
					skip = true;
				}
				else {
					if (*in_pos == '}') {
						// we got '}' instead of '{'
						error = error_code::fmt_bad;
					}
					break;
				}
			}
			if (out_pos < out_end) {
				// write byte
				*out_pos++ = *in_pos++;
				if(skip) in_pos++;
			}
			else {
				// stop (no space)
				error = error_code::buff_full;
				break;
			}
		}
		return error;
	}

	// parse the replacement field part of the fmt format string in to out
	inline error_code parse_fmt_repl_field(
		unsigned char const*& in_pos, unsigned char const* in_end,
		buff_span_const& field_name,
		buff_span_const& format_spec) noexcept
	{
		FSTLOG_ASSERT(in_pos != nullptr && in_pos < in_end && *in_pos == '{');
		in_pos++;
		bool name_set = false;
		const auto name_begin = in_pos;
		auto name_end = name_begin;
		auto spec_begin = in_pos;
		auto spec_end = spec_begin;
		error_code error = error_code::fmt_bad;
		while (in_pos < in_end) {
			if (*in_pos == '}') {
				if (!name_set) {
					name_end = in_pos;
					spec_begin = in_pos;
				}
				spec_end = in_pos++;
				error = error_code::none;
				break;
			}
			else if (*in_pos == '{') {
				// error unmatched '{'
				break;
			}
			else if (!name_set && *in_pos == ':') {
				name_end = in_pos;
				spec_begin = in_pos + 1;
				name_set = true;
			}
			in_pos++;
		}

		if (error != error_code::none) {
			name_end = name_begin;
			spec_begin = spec_end;
		}
		field_name = buff_span_const(name_begin, static_cast<std::size_t>(name_end - name_begin));
		format_spec = buff_span_const(spec_begin, static_cast<std::size_t>(spec_end - spec_begin));
		return error;
	}
    
    inline const unsigned char* skip_align(
		const unsigned char* begin, 
		const unsigned char* end) noexcept 
	{
        if (begin >= end) return begin;
        const auto ch{ *begin };
        const auto char_bytes = utf8_bytes(ch);
        if (char_bytes == 0) return begin;

		// check if there is a fill char (possibly utf8) + align char
        if (end - begin > char_bytes) {
            const auto align{ *(begin + char_bytes) };
            if (align == '<' ||
                align == '>' ||
                align == '^')
            {
                // skip fill char + align char
				return begin + char_bytes + 1;
            }
        }
		// if there was no fill char + align char
		// check if there is a single align char
        if (ch == '<' ||
            ch == '>' ||
            ch == '^')
        {
            // skip single align char
			return begin + 1;
        }
		// nothing to skip
        return begin;
    }

	inline const unsigned char* skip_sign_alt_0(
		const unsigned char* begin,
		const unsigned char* end) noexcept
	{
		auto out = begin;
		if (out < end && ((*out == '+') | (*out == '-') | (*out == ' '))) out++;
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

    inline tz_format get_zone(
		const unsigned char*& begin, 
		const unsigned char* end) noexcept 
	{
        tz_format out{ tz_format::Local };
        if (begin < end) {
            if (*begin == 'L') {
                begin++;
            }
            else if (*begin == 'U') {
                out = tz_format::UTC;
                begin++;
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
		buff_span_const& form_spec, 
		buff_span_const& time_fmt) noexcept 
	{
		auto error = error_code::none;
		if (form_spec.empty()) {
			time_fmt = form_spec;
			return error;
		}
		const auto begin = form_spec.data();
		const auto end = begin + form_spec.size_bytes();
		auto pos = begin;
		pos = skip_align(pos, end);
		const auto pos_0{ pos };
		pos = skip_sign_alt_0(pos, end);
		if (pos != pos_0) {
			// "[sign][#][0] not supported in timestamp formatting (format string)!"
			error = error_code::fmt_bad;
		}
		pos = skip_numbers(pos, end);
		form_spec = buff_span_const{ begin, static_cast<std::size_t>(pos - begin) };
		time_fmt = buff_span_const{ pos, static_cast<std::size_t>(end - pos) };
		return error;
	}

	inline logfield get_repl_field_id(buff_span_const name) noexcept {
		auto str_v = std::string_view{
			safe_reinterpret_cast<const char*>(name.data()), name.size_bytes() };
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
		case logfield::Channel:	return "channel";
		case logfield::Timestamp: return "timestamp";
		case logfield::Thread: return "thread";
		case logfield::Logger: return "logger";
		case logfield::File: return "file";
		case logfield::Line: return "line";
		case logfield::Function: return "function";
		case logfield::Message:	return "message";
		default: return "invalid";
		}
	}

	inline bool valid_format_spec(buff_span_const format_spec) {
		if (format_spec.empty()) return true;
		auto pos = format_spec.data();
		const auto end = pos + format_spec.size_bytes();
		// fill-align
		const auto align_char = pos;
		pos = skip_align(pos, end);
		if (pos - align_char > 1) {
			auto bytes = valid_utf8(align_char, pos);
			if (bytes != pos - align_char - 1) return false;
			if (!printable_utf8(align_char, bytes)) return false;
			if (*align_char == '{' || *align_char == '}') return false;
		}
		// sign '#' '0'
		pos = skip_sign_alt_0(pos, end);
		// width
		const auto num_begin = pos;
		pos = skip_numbers(pos, end);
		if (pos - num_begin > 0) {
			if (*num_begin == '0') return false;
			if (pos - num_begin > 4) return false;
		}
		// precision
		if (pos < end && *pos == '.') {
			pos++;
			const auto prec_begin = pos;
			pos = skip_numbers(pos, end);
			if (pos - prec_begin > 0) {
				if (*prec_begin == '0') return false;
				if (pos - prec_begin > 4) return false;
			}
			else {
				return false;
			}
		}
		// 'L' local number separator
		if (pos < end && *pos == 'L') pos++;
		// type
		if (pos == end) return true;
		if (end - pos != 1) return false;
		// "a" | "A" | "b" | "B" | "c" | "d" | "e" | "E" | "f" | "F" | "g" | "G" | "o" | "p" | "s" | "x" | "X"
		if (*pos == 'a' || *pos == 'A' || *pos == 'b' || *pos == 'B' || *pos == 'c' || *pos == 'd'
			|| *pos == 'e' || *pos == 'E' || *pos == 'f' || *pos == 'F' || *pos == 'g' || *pos == 'G'
			|| *pos == 'o' || *pos == 'p' || *pos == 's' || *pos == 'x' || *pos == 'X')
		{
			return true;
		}
		return false;
	}
}
