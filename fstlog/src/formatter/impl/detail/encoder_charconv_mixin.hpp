//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <charconv>
#include <cstddef>
#include <limits>
#include <string_view>
#include <type_traits>

#include <detail/unaligned_span.hpp>
#include <fstlog/detail/error_code.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <detail/utf_conv.hpp>
#include <formatter/impl/detail/encoder_helper.hpp>
#include <formatter/impl/detail/format_setting_txt.hpp>
#include <formatter/impl/detail/shift_fill.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/convert_to_basic_string_view.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/is_char_type.hpp>
#include <fstlog/detail/is_string_like.hpp>
#include <fstlog/detail/log_type_metadata.hpp>
#include <fstlog/detail/str_hash_fnv.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
    template<typename L>
    class encoder_charconv_mixin : public L {
    public:
        using memory_resource_type = typename L::memory_resource_type;
        typedef format_setting_txt format_type;

        explicit encoder_charconv_mixin(memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<L, memory_resource_type*>)
            : L(resource) {}

        encoder_charconv_mixin(const encoder_charconv_mixin& other) noexcept(
            noexcept(other.get_memory_resource())
            && std::is_nothrow_constructible_v<
                encoder_charconv_mixin,
                const encoder_charconv_mixin&,
                memory_resource_type*>)
            : encoder_charconv_mixin(other, other.get_memory_resource()) {}
        encoder_charconv_mixin(const encoder_charconv_mixin& other, memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<
                L,
                const encoder_charconv_mixin&,
                memory_resource_type*>)
            : L(other, resource) {}

        encoder_charconv_mixin(encoder_charconv_mixin&& other) = delete;
        encoder_charconv_mixin& operator=(const encoder_charconv_mixin& rhs) = delete;
        encoder_charconv_mixin& operator=(encoder_charconv_mixin&& rhs) = delete;

        ~encoder_charconv_mixin() = default;
        
        // integral
        template<typename T, std::enable_if_t<
            std::is_integral_v<T> &&
            !std::is_same_v<rm_cvref_t<T>, bool> &&
            !is_char_type_v<T>
        >* = nullptr>
        void encode(T data, format_type format) noexcept {
            // ensure minimum space for sign + prefix
            if (!this->output_has_space(3)) {
                this->set_error(__FILE__, __LINE__, error_code::buff_full);
                return;
            }
            const auto str_begin = this->output_ptr();
            // We will increase buffer_pos as we write the formatted data
            auto buffer_pos = str_begin;
            
            // write the sign
            detail::write_sign(format.sign, data < 0, buffer_pos);
            
            // format type
            const unsigned char type_char = detail::sanitize_int_type_char(format.type);

            // set base
            const int base = detail::get_int_base(type_char);

            // write the prefix 
            // do not write prefix if data is octal 0 (write 0 not 00)
            if (format.alternate
                && !(type_char == 'o' && data == 0))
            {
                detail::int_write_prefix(base, type_char, buffer_pos);
            }
            // convert data to unsigned absolute value
            const std::make_unsigned_t<T> abs_data = detail::abs_unsigned(data);
            // use std::to_chars() to format the number
            char* const digits_start = safe_reinterpret_cast<char*>(buffer_pos);
            auto buffer_end = this->output_end();
            auto result = std::to_chars(
                digits_start,
                safe_reinterpret_cast<char*>(buffer_end),
                abs_data,
                base);
            if (result.ec != std::errc{}) {
                this->set_error(__FILE__, __LINE__, error_code::buff_full);
                return;
            }
            auto str_end = safe_reinterpret_cast<unsigned char*>(result.ptr);
            
            // convert digits to upper case (if format type is X, binary B can't have chars )
            if (type_char == 'X') {
                detail::num_to_upper_case(digits_start, result.ptr);
            }

            // apply alligning and filling with fill_char
            if (format.width != 0) {
                //setting default alignment for integrals
                if (format.align == 0) format.align = '>';
                const auto str_len{ static_cast<std::size_t>(str_end - str_begin) };
                auto result_len = detail::shift_fill(
                    { str_begin, static_cast<std::size_t>(buffer_end - str_begin) },
                    { str_len, str_len },
                    format);
                str_end = str_begin + result_len.byte_len;
            }
            // update buffer pointer to the first free byte
            this->set_output_ptr_unchecked(str_end);
        }

        // float
        template<typename T, std::enable_if_t<
            std::is_floating_point_v<T>>* = nullptr>
        void encode(T data, format_type format) noexcept {
            const auto str_begin = this->output_ptr();
            // We will increase buffer_pos as we write the formatted data
            auto buffer_pos = str_begin;
            const auto buffer_end = this->output_end();
            // ensure minimum space for sign
            if (buffer_pos >= buffer_end) {
                this->set_error(__FILE__, __LINE__, error_code::buff_full);
                return;
            }
            // write the sign
            detail::write_sign(format.sign, data < 0, buffer_pos);
            
            // we write the abs(data), sign is already taken care of
            if (data < 0) data = -data;

            // format type
            const unsigned char type_char = detail::sanitize_float_type_char(format.type);

            const auto fmt = std::chars_format(detail::charconv_float_format(type_char));
            
            const auto num_begin = safe_reinterpret_cast<char*>(buffer_pos);
            std::to_chars_result result;
            // default precision (no precision specified)
            if (format.precision == 0xffff) {
                result = std::to_chars(
                    num_begin, 
                    safe_reinterpret_cast<char*>(buffer_end),
                    data, 
                    fmt);
            }
            else {
                result = std::to_chars(
                    num_begin, 
                    safe_reinterpret_cast<char*>(buffer_end),
                    data, 
                    fmt, 
                    format.precision);
            }

            if (result.ec != std::errc{}) {
                this->set_error(__FILE__, __LINE__, error_code::buff_full);
                return;
            }

            // upper_case
            if (fmt != std::chars_format::fixed && format.type <= 'G') {
                detail::num_to_upper_case(num_begin, result.ptr);
            }
            
            unsigned char* str_end = safe_reinterpret_cast<unsigned char*>(result.ptr);
            
            // apply fill align if needed
            if (format.width != 0) {
                //setting default alignment for floats
                if (format.align == 0) format.align = '>';
                const auto str_len{ static_cast<std::size_t>(str_end - str_begin) };
                auto result_len = detail::shift_fill(
                    { str_begin, static_cast<std::size_t>(buffer_end - str_begin) },
                    { str_len, str_len },
                    format);
                str_end = str_begin + result_len.byte_len;
            }
            // update buffer pointer to the first free byte
            this->set_output_ptr_unchecked(str_end);
        }

        // char
        void encode(char data, format_type format) noexcept {
            const unsigned char byte = *safe_reinterpret_cast<const unsigned char*>(&data);
            // safe ASCII and formatted textually
            if ((format.type == 0 || format.type == 'c')
                && byte < 0x80 && detail::utf::safe_utf_code_point(byte))
            {
                encode(byte_span_const{ &byte, 1 }, format);
            }
            // invalid, unsafe or formatted numerically
            else {
                if (format.type == 0 || format.type == 'c') {
                    format.type = 'x';
                    format.alternate = true;
                }
                encode(byte, format);
            }
        }

        // char16_t, char32_t
        template<typename T, std::enable_if_t<
            std::is_same_v<T, char16_t>
            || std::is_same_v<T, char32_t>
        >* = nullptr>
        void encode(T data, format_type format) noexcept {
            if ((format.type == 0 || format.type == 'c')
                && detail::utf::safe_utf_code_point(data))
            {
                encode(unaligned_span<const T>{ &data, 1 }, format);
            }
            else {
                if (format.type == 0 || format.type == 'c') {
                    format.type = 'x';
                    format.alternate = true;
                }
                if constexpr (std::is_same_v<T, char32_t>) {
                    encode(static_cast<std::uint_least32_t>(data), format);
                }
                else {
                    encode(static_cast<std::uint_least16_t>(data), format);
                }
            }
        }

        // string in buffer
        template<typename T, std::enable_if_t<
            std::is_same_v<std::remove_const_t<T>, unsigned char>
            || (is_char_type_v<T> &&
                !std::is_same_v<std::remove_const_t<T>, char>)
            >* = nullptr>
        void encode(unaligned_span<const T> data, format_type format) noexcept {
            if (data.empty()) return;
            constexpr int bit_size{ sizeof(T) * 8 };
            unaligned_span output(
                this->output_ptr(),
                static_cast<std::size_t>(this->output_end() - this->output_ptr()));
            const auto result = detail::utf::utf8conv<bit_size>(data, output, format.precision);
            if (result.ec != error_code::none) {
                this->set_error(__FILE__, __LINE__, result.ec);
            }
            
            // fill align
            if (format.width != 0) {
                const auto byte_size = static_cast<std::size_t>(output.data_bytes() - this->output_ptr());
                auto result_len = detail::shift_fill(
                    { this->output_ptr(), static_cast<std::size_t>(this->output_end() - this->output_ptr()) },
                    {byte_size, result.codepoints_written },
                    format);
                this->advance_output_unchecked(result_len.byte_len);
            }
            // no fill align
            else {
                this->set_output_ptr_unchecked(output.data_bytes());
            }
        }

        // string_view
        template<typename T>
        void encode(std::basic_string_view<T> strv, format_type format) noexcept {
            if (strv.empty()) return;
            if constexpr (std::is_same_v<std::remove_const_t<T>, char>) {
                encode(unaligned_span<const unsigned char>{
                    safe_reinterpret_cast<const unsigned char*>(strv.data()),
                        strv.size() }
                , format);
            }
            else {
                encode(unaligned_span<const T>{ strv.data(), strv.size() }, format);
            }
        }

        // pointer
        template<typename T, std::enable_if_t<
            std::is_pointer_v<std::remove_reference_t<T>>
            >* = nullptr>
        void encode(T data, format_type format) noexcept {
            format.type = 'x';
            format.alternate = true;
            encode(safe_reinterpret_cast<std::uintptr_t>(data), format);
        }

        // bool
        template<typename T, std::enable_if_t<
            std::is_same_v<rm_cvref_t<T>, bool>
            >* = nullptr>
        void encode(T data, format_type format) noexcept {
            if (format.type == 0 || format.type == 's') {
                if (data) {
                    encode(std::string_view{ "true" }, format);
                }
                else {
                    encode(std::string_view{ "false" }, format);
                }
            }
            else {
                encode(static_cast<int>(data), format);
            }
        }

        // str_hash_fnv
        template<typename T, std::enable_if_t<
            std::is_same_v<rm_cvref_t<T>, str_hash_fnv>
            >* = nullptr>
        void encode(T data, format_type format) noexcept {
            const auto str = this->convert_hash(data);
            if (!str.empty()) {
                encode(str, format);
            }
            else {
                char fmt_type{ static_cast<char>(format.type) };
                if (fmt_type == 0 || fmt_type == 's') {
                    format.type = 'x';
                    format.alternate = true;
                }
                encode(data.hash_, format);
            }
        }

        void reencode_tail_string(unsigned char* str_begin, format_type format) {
            FSTLOG_ASSERT(str_begin >= this->output_begin() 
                && str_begin <= this->output_ptr());
            if (format.width == 0 && format.precision == 0xffff ) return;      
            const auto str_len = detail::utf::utf8_str_trim(
                byte_span_const{ str_begin, static_cast<std::size_t>(this->output_ptr() - str_begin) },
                format.precision);
            const auto result_len = detail::shift_fill(
                byte_span{ str_begin, static_cast<std::size_t>(this->output_end() - str_begin) },
                str_len,
                format);
            this->set_output_ptr_unchecked(str_begin + result_len.byte_len);
        }
    };
}
