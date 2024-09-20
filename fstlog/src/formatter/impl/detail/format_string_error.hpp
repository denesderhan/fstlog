//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>

#include <detail/byte_span.hpp>
#include <formatter/impl/detail/format_str_helper.hpp>
#include <formatter/impl/detail/valid_strftime_string.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
	inline const char* replacement_field_error(buff_span_const repl_field) {
		const auto field = get_repl_field_id(argument_name(repl_field));
		if (field == logfield::Invalid) return "Invalid field name in format string!";
		auto pos = repl_field.data();
		const auto end = pos + repl_field.size_bytes();
		//{fieldname:} or {}
		if (pos++ >= end) return nullptr;
		pos = skip_align(pos, end);
		const auto pos_0{ pos };
		pos = skip_sign_alt_0(pos, end);
		if (field == logfield::Timestamp
			&& pos != pos_0) return "[sign][#][0] not supported in timestamp formatting (format string)!";

		//skip width
		const auto temp_pos{ pos };
		pos = skip_numbers(pos, end);
		if (pos > temp_pos && *temp_pos == '0')
			return "[width] begins with '0' in a replacement field in format string!";
		if (pos - temp_pos > 4) return "Maximum [width] is 9999 in the replacement field in format string!";
		//skip precision
		if (pos < end && *pos == '.') {
			pos++;
			const auto pos2 = skip_numbers(pos, end);
			if (pos2 == pos) return "Missing ['.'precision] in a replacement field after '.' in format string!";
			if (*pos == '0' && pos2 - pos != 1) return "['.'precision] begins with '0' in a replacement field in format string!";
			if (field == logfield::Timestamp) {
				if (pos2 - pos != 1) return "Maximum ['.'precision] is 9 for timestamp field in format string!";
			}
			else {
				if (pos2 - pos > 4) return "Maximum ['.'precision] is 9999 in the replacement field in format string!";
			}
			pos = pos2;
		}

		if (pos < end) {
			if (field == logfield::Timestamp) {
				tz_format t_zone = get_zone(pos, end);
				const bool valid_strf = detail::valid_strftime_string(
					buff_span_const{ pos, static_cast<std::size_t>(end - pos) },
					t_zone);
				if (!valid_strf) return "Invalid strftime string in timestamp field in format string!";
			}
			else {
				//skip 'L'
				if (*pos == 'L') pos++;
				if (end - pos > 1) return "[type] in replacement field has to be 1 char long in format string!";
				if (pos < end) {
					const unsigned char data_type{ *pos };
					auto valid_types{ "aAbBcdeEfFgGopsxX" };
					auto tbl_ptr{ valid_types };
					while (*tbl_ptr != 0 && *tbl_ptr != data_type) tbl_ptr++;
					if (*tbl_ptr == 0) return "Invalid [type] in replacement field in format string!";
				}
			}
		}
		return nullptr;
	}

    inline const char* format_string_error(buff_span_const str) noexcept
    {
        if (str.empty() || str.data() == nullptr) return "Format string empty!";
        bool opened{ false };
        auto data{ str.data() };
        auto const end{ data + str.size_bytes() };
        const unsigned char* form_spec_begin{ 0 };
        while (data < end) {
			const auto bytes{ valid_utf8(data, end) };
			if (bytes == 0) return "Invalid utf8 character in format string!";
			if (!printable_utf8(data, bytes)) return "Unprintable character in format string!";

            if (bytes == 1) {
                auto ch = *data++;
                if (ch == '{') {
                    if (opened) {
						return "Double opening character '{' in format string!";
                    }
                    else if (data < end && *data == '{') {
                        data++; //escaping
                    }
                    else {
                        opened = true;
                        form_spec_begin = data;
                    }
                }
                else if (ch == '}') {
                    if (opened) {
                        opened = false;
						const buff_span_const repl_field{
							form_spec_begin, 
							static_cast<std::size_t>((data - 1) - form_spec_begin) };
						const auto error = replacement_field_error(repl_field);
						if(error != nullptr) return error;
                    }
                    else if (data < end && *data == '}') {
                        data++; //escaping
                    }
                    else {
                        return "Double closing character '}' in format string!";
                    }
                }
            }
            else {
                data += bytes;
            }
        }
		if (opened) return "Missing closing character '}' in format string!";
        return nullptr;
    }
}
