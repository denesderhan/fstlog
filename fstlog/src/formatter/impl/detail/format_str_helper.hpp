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
    
    inline buff_span_const get_replacement_field(
        buff_span_const& str) noexcept
    {
		FSTLOG_ASSERT(str.data() != nullptr);
		if (str.size_bytes() == 0) {
            return str;
        }
        auto pos = str.data() + 1;
        auto end_pos = str.data() + str.size_bytes();
        auto repl_begin = pos;
        while (pos != end_pos 
            && *pos != '{'
            && *pos != '}') 
        {
            pos++;
        }
        auto repl_end = pos;
        //replacement field end marker not missing (skip it)
        if (repl_end != end_pos) {
            pos++;    
        }
        str = buff_span_const{ pos, static_cast<std::size_t>(end_pos - pos) };
        return buff_span_const{ repl_begin, static_cast<std::size_t>(repl_end - repl_begin) };
    }

    inline buff_span_const format_spec(
        buff_span_const replacement_field) noexcept
    {
        const unsigned char* pos = replacement_field.data();
        const unsigned char* const end = pos + replacement_field.size_bytes();
        while (pos < end) {
            if (*pos++ == ':') break;
        }
        return buff_span_const{ pos, static_cast<std::size_t>(end - pos) };
    }

    inline buff_span_const argument_name(
        buff_span_const& replacement_field) noexcept
	{
        const unsigned char* const begin = replacement_field.data();
        const unsigned char* const end = begin + replacement_field.size_bytes();
        const unsigned char* pos = begin;
        while (pos < end) {
			if (*pos == ':') break;
			pos++;
        }
		replacement_field = buff_span_const{ pos, static_cast<std::size_t>(end - pos) };
        return buff_span_const{ begin, static_cast<std::size_t>(pos - begin) };
    }
    
    inline const unsigned char* skip_align(
		const unsigned char* begin, 
		const unsigned char* end) noexcept 
	{
        if (begin >= end) return begin;
        const auto ch{ *begin };
        const auto char_bytes = utf8_bytes(ch);
        if (char_bytes == 0) return begin;

        if (end - begin > char_bytes) {
            const auto align{ *(begin + char_bytes) };
            if (align == '<' ||
                align == '>' ||
                align == '^')
            {
                return begin + char_bytes + 1;
            }
        }
        if (ch == '<' ||
            ch == '>' ||
            ch == '^')
        {
            return begin + 1;
        }
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

	inline logfield get_repl_field_id(
		buff_span_const name) noexcept
	{
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
}
