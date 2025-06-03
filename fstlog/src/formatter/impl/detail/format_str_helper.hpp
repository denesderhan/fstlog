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
#include <formatter/impl/detail/uint_fromchars_4digit.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
		
	// write the text part of the fmt format string in to out
	inline error_code parse_fmt_text(
		unsigned char const*& in_pos, unsigned char const* in_end,
		unsigned char*& out_pos, unsigned char const* out_end) noexcept
	{
		FSTLOG_ASSERT(in_pos != nullptr && out_pos != nullptr);
		error_code error = error_code::none;
		auto out_beg = out_pos;
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
		sanitize_utf8_str(out_beg, out_pos);
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
		// chek if there is a single align char
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
    
    inline std::uint16_t get_width(
		const unsigned char*& begin, 
		const unsigned char* end) noexcept 
	{
		std::uint16_t width{ 0 };
		//if error happens width is not changed (default)
		uint_fromchars_4digit(width, begin, end);
		return width;
    }

    inline std::uint16_t get_precision(
		const unsigned char*& begin, 
		const unsigned char* end) noexcept 
	{
		//special case for indicating default value
		std::uint16_t num{ 0xffff };
		if (begin != end && *begin == '.') {
            begin++;
			uint_fromchars_4digit(num, begin, end);
        }
		return num;
    }

	inline buff_span_const time_format(buff_span_const& form_spec) noexcept {
		auto begin = form_spec.data();
		auto end = begin + form_spec.size_bytes();
		auto pos = skip_numbers(skip_align(begin, end), end);
		form_spec = buff_span_const{ begin, static_cast<std::size_t>(pos - begin) };
		return buff_span_const{ pos, static_cast<std::size_t>(end - pos) };
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
}
