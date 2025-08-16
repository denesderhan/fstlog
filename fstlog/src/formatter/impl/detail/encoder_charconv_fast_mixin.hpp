//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string_view>
#include <type_traits>

#include <detail/unaligned_span.hpp>
#include <fstlog/detail/error_code.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <detail/utf_conv.hpp>
#include <formatter/impl/detail/encoder_helper.hpp>
#include <formatter/impl/detail/format_setting_txt_fast.hpp>
#include <fstlog/detail/convert_to_basic_string_view.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/is_char_type.hpp>
#include <fstlog/detail/is_string_like.hpp>
#include <fstlog/detail/log_type_metadata.hpp>
#include <fstlog/detail/str_hash_fnv.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
    template<typename L>
    class encoder_charconv_fast_mixin : public L {
    public:
        using allocator_type = typename L::allocator_type;
        typedef format_setting_txt_fast format_type;

        encoder_charconv_fast_mixin() noexcept(
            noexcept(allocator_type())
            && noexcept(encoder_charconv_fast_mixin(allocator_type{})))
            : encoder_charconv_fast_mixin(allocator_type{}) {}
        explicit encoder_charconv_fast_mixin(allocator_type const& allocator) noexcept(
            noexcept(L(allocator_type{})))
            : L(allocator) {}

        encoder_charconv_fast_mixin(const encoder_charconv_fast_mixin& other) noexcept(
            noexcept(encoder_charconv_fast_mixin::get_allocator())
            && noexcept(encoder_charconv_fast_mixin(encoder_charconv_fast_mixin{}, allocator_type{})))
            : encoder_charconv_fast_mixin(other, other.get_allocator()) {}
        encoder_charconv_fast_mixin(const encoder_charconv_fast_mixin& other, allocator_type const& allocator) noexcept(
            noexcept(L(encoder_charconv_fast_mixin{}, allocator_type{})))
            : L(other, allocator) {}

        encoder_charconv_fast_mixin(encoder_charconv_fast_mixin&& other) = delete;
        encoder_charconv_fast_mixin& operator=(const encoder_charconv_fast_mixin& rhs) = delete;
        encoder_charconv_fast_mixin& operator=(encoder_charconv_fast_mixin&& rhs) = delete;

        ~encoder_charconv_fast_mixin() = default;

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
            auto buffer_pos{ str_begin };
            
            // write sign
            if (data < 0) *buffer_pos++ = '-';
            
            // format type
            const unsigned char type_char = detail::sanitize_int_type_char(format.type);

            // set base
            const int base = detail::get_int_base(type_char);
            
            // write the prefix 
            // do not write prefix if data is octal 0 (write 0 not 00)
            if (!(type_char == 'o' && data == 0)) {
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

            // update buffer pointer to the first free byte
            this->set_output_ptr_unchecked(str_end);
        }
        
        // float
        template<typename T, std::enable_if_t<
            std::is_floating_point_v<T>>* = nullptr>
        void encode( T data, format_type format ) noexcept {
            auto str_beg = safe_reinterpret_cast<char*>(this->output_ptr());
            auto buffer_end = safe_reinterpret_cast<char*>(this->output_end());

            // format type
            const unsigned char type_char = detail::sanitize_float_type_char(format.type);

            const auto fmt = std::chars_format(detail::charconv_float_format(type_char));
              
            std::to_chars_result result;
            if (format.precision == 0xffff) {
                result = std::to_chars(str_beg, buffer_end, data, fmt);
            }
            else {
                result = std::to_chars(str_beg, buffer_end, data, fmt, format.precision);
            }

            if (result.ec != std::errc{}) {
                this->set_error(__FILE__, __LINE__, error_code::buff_full);
                return;
            }
            
            // update buffer pointer to the first free byte
            this->set_output_ptr_unchecked(safe_reinterpret_cast<unsigned char*>(result.ptr));
        }
        
        // char
        void encode(char data, format_type format) noexcept {
            const unsigned char byte = *safe_reinterpret_cast<const unsigned char*>(&data);
            // safe ASCII and formatted textually
            if ((format.type == 0 || format.type == 'c')
                && byte < 0x80 && detail::utf::safe_utf_code_point(byte))
            {
                if (this->output_has_space()) {
                    *this->output_ptr() = byte;
                    this->advance_output_unchecked(1);
                }
                else {
                    this->set_error(__FILE__, __LINE__, error_code::buff_full);
                }
            }
            // invalid, unsafe or formatted numerically
            else {
                if (format.type == 0 || format.type == 'c') {
                    format.type = 'x';
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
        void encode(unaligned_span<const T> data, [[maybe_unused]] format_type format) noexcept {
            if (data.empty()) return;
            constexpr int bit_size{ sizeof(T) * 8 };
            unaligned_span output(
                this->output_ptr(), 
                static_cast<std::size_t>(this->output_end() - this->output_ptr()));
            const auto result = detail::utf::utf8conv<bit_size>(data, output);
            this->set_output_ptr_unchecked(output.data_bytes());
            if (result.ec != error_code::none) {
                this->set_error(__FILE__, __LINE__, result.ec);
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
                if (format.type == 0
                    || format.type == 's')
                {
                    format.type = 'x';
                }
                encode(data.hash_, format);
            }
        }

        static void reencode_tail_string(
            [[maybe_unused]] unsigned char* str_begin, 
            [[maybe_unused]] format_type format) noexcept {}
    };
}
