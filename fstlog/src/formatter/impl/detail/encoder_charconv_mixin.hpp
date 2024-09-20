//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <charconv>
#include <cstddef>
#include <cstring>
#include <limits>
#include <string_view>
#include <type_traits>
#pragma intrinsic(memmove)

#include <detail/byte_span.hpp>
#include <detail/utf_conv.hpp>
#include <detail/error_code.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/convert_to_basic_string_view.hpp>
#include <fstlog/detail/is_char_type.hpp>
#include <fstlog/detail/is_string_like.hpp>
#include <fstlog/detail/log_type_metadata.hpp>
#include <fstlog/detail/types.hpp>
#include <fstlog/detail/str_hash_fnv.hpp>
#include <fstlog/detail/ut_cast.hpp>
#include <formatter/impl/detail/format_setting_txt.hpp>
#include <formatter/impl/detail/shift_fill.hpp>
#include <formatter/impl/detail/time_to_str_converter.hpp>

namespace fstlog {
    template<typename L>
    class encoder_charconv_mixin : public L,
        public time_to_str_converter
    {
    public:
        using allocator_type = typename L::allocator_type;
        typedef format_setting_txt format_type;

        encoder_charconv_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(encoder_charconv_mixin(allocator_type{})))
			: encoder_charconv_mixin(allocator_type{}) {}
		explicit encoder_charconv_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
            : L(allocator) {}

        encoder_charconv_mixin(const encoder_charconv_mixin& other) noexcept(
			noexcept(encoder_charconv_mixin::get_allocator())
			&& noexcept(encoder_charconv_mixin(encoder_charconv_mixin{}, allocator_type{})))
            : encoder_charconv_mixin(other, other.get_allocator()) {}
        encoder_charconv_mixin(const encoder_charconv_mixin& other, allocator_type const& allocator) noexcept(
			noexcept(L(encoder_charconv_mixin{}, allocator_type{}))
			&& noexcept(time_to_str_converter(encoder_charconv_mixin{})))
            : L(other, allocator),
            time_to_str_converter(other) {}

        encoder_charconv_mixin(encoder_charconv_mixin&& other) = delete;
        encoder_charconv_mixin& operator=(const encoder_charconv_mixin& rhs) = delete;
        encoder_charconv_mixin& operator=(encoder_charconv_mixin&& rhs) = delete;

        ~encoder_charconv_mixin() = default;

        //integral
        template<typename T, std::enable_if_t<
            std::is_integral_v<T> &&
            !std::is_same_v<rm_cvref_t<T>, bool> &&
            !is_char_type_v<T>
            >* = nullptr>
        void encode(T data, format_type format) noexcept {
            std::to_chars_result result;
            constexpr bool is_unsigned{ std::is_unsigned_v<T> };
            const auto str_begin = this->output_ptr();
            const auto buffer_end = this->output_end();
            const auto buff_end = safe_reinterpret_cast<char*>(buffer_end);
            auto pos{ str_begin };

            //[10(/0), 2(B,b), 10(d), 10(?), 16(X,x), 10(?), 10(?), 8(o)]
            alignas(constants::cache_ls_nosharing) const std::array<int, 8> base_table{ 
                10, 2, 10, 10, 16, 10, 10, 8 };
            const int base = base_table[(format.type >> 1) & 0b111];
            
            if (base == 10 || !format.alternate)
            {
                if (format.sign != '-' && (is_unsigned || data >= 0)) {
                    if (pos < buffer_end) {
                        *pos++ = format.sign;
                    }
                    else {
                        this->set_error(__FILE__, __LINE__, error_code::no_space_in_buffer);
                        return;
                    }
                }
                auto str_beg = safe_reinterpret_cast<char*>(pos);
                result = std::to_chars(str_beg, buff_end, data, base);
                if (result.ec == std::errc{}) {
                    pos = safe_reinterpret_cast<unsigned char*>(result.ptr);
                }
                else {
                    this->set_error(__FILE__, __LINE__, error_code::no_space_in_buffer);
                    return;
                }
            }
            //!base10 && alternate
            else {
                if (buffer_end - str_begin >= 3) {
                    const std::array<unsigned char, 4> prefix_tbl{ 'b', '0', 'x', '!' };
                    const auto type_char{ prefix_tbl[static_cast<unsigned int>(base) >> 3] };
                    if constexpr (is_unsigned) {
                        if (format.sign != '-') *pos++ = format.sign;
                        if (type_char != '0') {
                            *pos++ = '0';
                            *pos++ = type_char;
                        }
                        else if (data != 0) *pos++ = '0';
                        auto str_beg = safe_reinterpret_cast<char*>(pos);
                        result = std::to_chars(str_beg, buff_end, data, base);
                        //for upper_case
                        pos--;
                    }
                    else {
                        if (data >= 0) {
                            //compiler warning forces code duplication:
                            //conditional expression is constant, consider using 'if constexpr' statement instead
                            if (format.sign != '-') *pos++ = format.sign;
                            if (type_char != '0') {
                                *pos++ = '0';
                                *pos++ = type_char;
                            }
                            else if (data != 0) *pos++ = '0';
                            auto str_beg = safe_reinterpret_cast<char*>(pos);
                            result = std::to_chars(str_beg, buff_end, data, base);
                            //for upper_case
                            pos--;
                        }
                        else {
                            *pos = '-';
                            *(pos + 1) = '0';
                            pos = type_char != '0' ? pos + 2 : pos + 1;
                            auto str_beg = safe_reinterpret_cast<char*>(pos);
                            result = std::to_chars(str_beg, buff_end, data, base);
                            *pos = type_char;
                        }
                    }

                    if (result.ec == std::errc{}) {
                        //OK
                    }
                    else {
                        this->set_error(__FILE__, __LINE__, error_code::no_space_in_buffer);
                        return;
                    }
                    const bool upper_case{ format.type <= 'X' };
                    if (upper_case) {
                        if (base == 16) {
                            while (pos < safe_reinterpret_cast<unsigned char*>(result.ptr)) {
                                if (*pos >= 'a') *pos = *pos - 32;
                                pos++;
                            }
                        }
                        else if (base == 2) {
                            *pos = 'B';
                        }
                    }
                    pos = safe_reinterpret_cast<unsigned char*>(result.ptr);
                }
                else {
                    this->set_error(__FILE__, __LINE__, error_code::no_space_in_buffer);
                    return;
                }
            }
            this->set_output_ptr_unchecked(pos);

            if (format.width != 0) {
                //setting default alignment for integrals
                if (format.align == 0) format.align = '>';
                const auto byte_size{ pos - str_begin};
                auto sf_result = shift_fill(
                    str_begin,
                    buffer_end,
                    byte_size,
                    byte_size,
                    format.width,
                    format.align,
                    format.fill_char);
                this->set_output_ptr_unchecked(sf_result.ptr);
            }
        }

        //float
        template<typename T, std::enable_if_t<
            std::is_floating_point_v<T>>* = nullptr>
        void encode(T data, format_type format) noexcept {
            alignas(constants::cache_ls_nosharing)
                const std::array<std::chars_format, 8> ch_form_table{
                std::chars_format::general,
                std::chars_format::hex,
                std::chars_format::general,
                std::chars_format::general,
                std::chars_format::general,
                std::chars_format::scientific,
                std::chars_format::fixed,
                std::chars_format::general
            };
            const auto fmt = ch_form_table[ format.type & 0b111 ];
            
            auto str_beg = safe_reinterpret_cast<char*>(this->output_ptr());
            const auto buff_end = safe_reinterpret_cast<char*>(this->output_end());
            std::to_chars_result result;
            if (format.precision == 0xffff) {
                result = std::to_chars(str_beg, buff_end, data, fmt);
            }
            else {
                result = std::to_chars(str_beg, buff_end, data, fmt, format.precision);
            }

            if (result.ec != std::errc{}) {
                this->set_error(__FILE__, __LINE__, error_code::no_space_in_buffer);
                return;
            }

            unsigned char* str_end = safe_reinterpret_cast<unsigned char*>(result.ptr);
           
            const bool add_sign{ format.sign != '-' && *str_beg > '-' };

            //sign fix
            if (add_sign && safe_reinterpret_cast<unsigned char*>(buff_end) > str_end) {
                memmove(str_beg + 1, str_beg, str_end - safe_reinterpret_cast<unsigned char*>(str_beg));
                str_end++;
                *str_beg = format.sign;
            }

            //upper_case
            if (fmt != std::chars_format::fixed && format.type <= 'G') {
                auto pos{ safe_reinterpret_cast<unsigned char*>(str_beg) };
                while (pos < str_end) {
                    if (*pos >= 'a') *pos -= 32;
                    pos++;
                }
            }
                
            if (format.width == 0) {
                this->set_output_ptr_unchecked(str_end);
            }
            else {
                //setting default alignment for floats
                if (format.align == 0) format.align = '>';
                const auto byte_size{ str_end - safe_reinterpret_cast<unsigned char*>(str_beg) };
                auto sf_result = shift_fill(
                    safe_reinterpret_cast<unsigned char*>(str_beg),
                    safe_reinterpret_cast<unsigned char*>(buff_end),
                    byte_size,
                    byte_size,
                    format.width,
                    format.align,
                    format.fill_char);
                this->set_output_ptr_unchecked(sf_result.ptr);
            }
        }

        //char char8
        template<typename T, std::enable_if_t<
            is_char_type_v<T>
            && sizeof(T) == 1
            >* = nullptr>
        void encode(T data, format_type format) noexcept {
            if (this->output_has_space()) {
                const auto data_ptr = safe_reinterpret_cast<const unsigned char*>(&data);
                auto str_begin{ this->output_ptr() };
                *str_begin = *data_ptr;

                if (format.width <= 1) {
                    this->advance_output_unchecked(1);
                }
                else {
                    auto sf_result = shift_fill(
                        str_begin,
                        this->output_end(),
                        1,
                        1,
                        format.width,
                        format.align,
                        format.fill_char);
                    this->set_output_ptr_unchecked(sf_result.ptr);
                }
            }
            else {
                this->set_error(__FILE__, __LINE__, error_code::no_space_in_buffer);
            }
        }

        //utf16 utf32 character
        template<typename T, std::enable_if_t<
            is_char_type_v<T>
            && sizeof(T) != 1
            >* = nullptr>
        void encode(T data, format_type format) noexcept {
            encode(byte_span<T>{ &data, 1 }, format);
        }

        //string in buffer
        template<typename T, std::enable_if_t<
            is_char_type_v<T>
            || std::is_same_v<std::remove_const_t<T>, unsigned char>
            >* = nullptr>
        void encode(byte_span<T> data, format_type format) noexcept {
			FSTLOG_ASSERT(data.data() != nullptr);
            const auto out_begin{ this->output_ptr() };
            auto out_end{ this->output_end() };
            const unsigned char* str_begin{ data.data() };
            std::size_t byte_size = data.size_bytes();

            static_assert(std::numeric_limits<unsigned char>::digits == 8);
            constexpr int bit_size{ sizeof(T) * std::numeric_limits<unsigned char>::digits };
            std::size_t char_num{ format.precision };
            const auto result = detail::utf8conv<bit_size, T, unsigned char>(
                str_begin,
                str_begin + byte_size,
                out_begin,
                out_end,
                char_num);
            if (result.ec != error_code::none) {
                this->set_error(__FILE__, __LINE__, result.ec);
                return;
            }

            byte_size = static_cast<std::size_t>(result.ptr - out_begin);
            if (format.width > byte_size / 4) {
                auto sf_result = shift_fill(
                    out_begin,
                    out_end,
                    byte_size,
                    char_num,
                    format.width,
                    format.align,
                    format.fill_char);
                this->set_output_ptr_unchecked(sf_result.ptr);
            }
            else {
                this->set_output_ptr_unchecked(result.ptr);
            }
        }

        template<typename T>
        void encode(std::basic_string_view<T> strv, format_type format) noexcept {
			encode(byte_span<const T>{ strv.data(), strv.size() }, format);
        }

        //pointer
        template<typename T, std::enable_if_t<
            std::is_pointer_v<std::remove_reference_t<T>>
            >* = nullptr>
        void encode(T data, format_type format) noexcept {
            format.type = 'x';
            format.alternate = true;
            encode(safe_reinterpret_cast<std::uintptr_t>(data), format);
        }

        //bool
        template<typename T, std::enable_if_t<
            std::is_same_v<rm_cvref_t<T>, bool>
            >* = nullptr>
        void encode(T data, format_type format) noexcept {
            char fmt_type{ static_cast<char>(format.type) };
			if (fmt_type == 0 || fmt_type == 's') {
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

        //str_hash_fnv
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

        //nanosec_epoch
        template<typename T, std::enable_if_t<
            std::is_same_v<rm_cvref_t<T>, stamp_type>
            >* = nullptr>
        void encode(T data, format_type format) noexcept {
            const auto str_begin{ this->output_ptr() };
            const auto end{ this->output_end() };
            const auto result = timestamp_to_chars(
                data,
                str_begin,
                end
            );
            if (result.ec == error_code::none) {
                const std::size_t byte_size{ static_cast<std::size_t>(result.ptr - str_begin) };
                const std::size_t char_num = charnum_utf8(str_begin, result.ptr);
                if (format.width > byte_size / 4) {
                    auto sf_result = shift_fill(
                        str_begin,
                        end,
                        byte_size,
                        char_num,
                        format.width,
                        format.align,
                        format.fill_char);
                    this->set_output_ptr_unchecked(sf_result.ptr);
                }
                else {
                    this->set_output_ptr_unchecked(result.ptr);
                }
            }
            else {
                this->set_error(__FILE__, __LINE__, result.ec);
            }
        }

        void reencode_tail_string(unsigned char* str_begin, format_type format) {
            auto str_end{ this->output_ptr() };
           	FSTLOG_ASSERT(str_begin >= this->output_begin() && str_begin <= str_end);
			std::size_t char_num{ format.precision };
            str_end = utf8_str_trim(str_begin, str_end, char_num);
            FSTLOG_ASSERT(str_begin <= str_end);                        
            if (format.width > char_num) {
                std::size_t str_byte_size = static_cast<std::size_t>(str_end - str_begin);
                auto result = shift_fill(
                    str_begin,
                    this->output_end(),
                    str_byte_size,
                    char_num,
                    format.width,
                    format.align,
                    format.fill_char);
                str_end = result.ptr;
            }
            this->set_output_ptr_unchecked(str_end);
        }
    };
}
