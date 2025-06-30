//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>

#include <detail/byte_span.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <detail/utf_conv.hpp>
#include <formatter/impl/detail/logfield.hpp>
#include <formatter/impl/detail/tz_format.hpp>
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
		inline error_code parse_fmt_text(
			unsigned char const*& in_pos, unsigned char const* in_end,
			unsigned char*& out_pos, unsigned char const* out_end) noexcept
		{
			FSTLOG_ASSERT(in_pos != nullptr && out_pos != nullptr);
			// fast path (ASCII + no special {, })
			auto fast_end = in_pos;
			while (fast_end < in_end
				&& ((*fast_end >= 0x20) & (*fast_end < 0x7F)
					& (*fast_end != '}') & (*fast_end != '{')) != 0)
			{
				fast_end++;
			}
			auto fast_len = static_cast<std::size_t>(fast_end - in_pos);
			if (fast_len > static_cast<std::size_t>(out_end - out_pos)) fast_len = static_cast<std::size_t>(out_end - out_pos);
			memcpy(out_pos, in_pos, fast_len);
			in_pos += fast_len;
			out_pos += fast_len;

			error_code error = error_code::none;
			while (in_pos < in_end) {
				bool skip = false;
				if (*in_pos == '{' || *in_pos == '}') {
					if (in_end - in_pos > 1 && *(in_pos + 1) == *in_pos) {
						// skip the duplicated '{' or '}'
						skip = true;
					}
					else {
						// we got '}' instead of '{'
						if (*in_pos == '}') {
							error = error_code::fmt_bad;
						}
						// break if '{' OR '}' (end of text OR error)
						break;
					}
				}
				auto next_in_pos = in_pos;
				std::uint32_t code_point = detail::decode_utf8_char(next_in_pos, in_end);
				auto next_out_pos = detail::encode_safe_utf8_char(code_point, out_pos, out_end);
				if (next_out_pos == nullptr) {
					error = error_code::buff_full;
					break;
				}
				out_pos = next_out_pos;
				in_pos = next_in_pos;
				if (skip) in_pos++;
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

		inline const unsigned char* skip_fill_align(
			const unsigned char* begin,
			const unsigned char* end) noexcept
		{
			if (begin >= end) return begin;
			auto pos = begin;
			const auto first_char = detail::decode_utf8_char(pos, end);
			
			// check if there is a good fill char (first_char) + align char
			if (pos < end) {
				const unsigned char align_char{ *pos };
				if (align_char == '<' ||
					align_char == '>' ||
					align_char == '^')
				{
					if (first_char != '{' && first_char != '}'
						&& detail::safe_utf_code_point(first_char))
					{
						// skip align char (and fill char)
						return ++pos;
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
			pos = skip_fill_align(pos, end);
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

		inline bool valid_format_spec(buff_span_const format_spec) {
			if (format_spec.empty()) return true;
			auto pos = format_spec.data();
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
