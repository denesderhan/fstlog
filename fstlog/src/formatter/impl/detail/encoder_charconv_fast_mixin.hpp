//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <charconv>
#include <cstddef>
#include <cstring>
#include <limits>
#include <string_view>
#include <type_traits>
#pragma intrinsic(memcpy)

#include <detail/byte_span.hpp>
#include <detail/utf_conv.hpp>
#include <detail/error_code.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <fstlog/detail/convert_to_basic_string_view.hpp>
#include <fstlog/detail/is_char_type.hpp>
#include <fstlog/detail/is_string_like.hpp>
#include <fstlog/detail/log_type_metadata.hpp>
#include <fstlog/detail/types.hpp>
#include <fstlog/detail/str_hash_fnv.hpp>
#include <fstlog/detail/ut_cast.hpp>
#include <formatter/impl/detail/format_setting_txt_fast.hpp>
#include <formatter/impl/detail/time_to_str_converter.hpp>

namespace fstlog {
    template<typename L>
    class encoder_charconv_fast_mixin : public L,
        public time_to_str_converter
    {
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
			noexcept(L(encoder_charconv_fast_mixin{}, allocator_type{}))
			&& noexcept(time_to_str_converter(encoder_charconv_fast_mixin{})))
            : L(other, allocator),
            time_to_str_converter(other) {
        }

        encoder_charconv_fast_mixin(encoder_charconv_fast_mixin&& other) = delete;
        encoder_charconv_fast_mixin& operator=(const encoder_charconv_fast_mixin& rhs) = delete;
        encoder_charconv_fast_mixin& operator=(encoder_charconv_fast_mixin&& rhs) = delete;

        ~encoder_charconv_fast_mixin() = default;

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

            if (base == 10)
            {
                auto str_beg = safe_reinterpret_cast<char*>(pos);
                result = std::to_chars(str_beg, buff_end, data, base);
            }
            else {
                if (buffer_end - str_begin >= 3) {
                    const std::array<unsigned char, 4> prefix_tbl{ 'b', '0', 'x', '!' };
                    const auto type_char{ prefix_tbl[static_cast<unsigned int>(base) >> 3] };
                    if constexpr (is_unsigned) {
                        if (type_char != '0') {
                            *pos++ = '0';
                            *pos++ = type_char;
                        }
                        else if (data != 0) *pos++ = '0';
                        auto str_beg = safe_reinterpret_cast<char*>(pos);
                        result = std::to_chars(str_beg, buff_end, data, base);
                    }
                    else {
                        if (data >= 0) {
                            //compiler warning forces code duplication:
                            //conditional expression is constant, consider using 'if constexpr' statement instead
                            if (type_char != '0') {
                                *pos++ = '0';
                                *pos++ = type_char;
                            }
                            else if (data != 0) *pos++ = '0';
                            auto str_beg = safe_reinterpret_cast<char*>(pos);
                            result = std::to_chars(str_beg, buff_end, data, base);
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
                }
                else {
                    this->set_error(__FILE__, __LINE__, error_code::no_space_in_buffer);
                    return;
                }
            }
            if (result.ec == std::errc{}) {
                this->set_output_ptr_unchecked(safe_reinterpret_cast<unsigned char*>(result.ptr));
            }
            else {
                this->set_error(__FILE__, __LINE__, error_code::no_space_in_buffer);
                return;
            }
        }
        
        //float
        template<typename T, std::enable_if_t<
            std::is_floating_point_v<T>>* = nullptr>
        void encode( T data, format_type format ) noexcept {
            auto str_beg = safe_reinterpret_cast<char*>(this->output_ptr());
            auto buff_end = safe_reinterpret_cast<char*>(this->output_end());

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
              
            std::to_chars_result result;
            if (format.precision == 0xffff) {
                result = std::to_chars(str_beg, buff_end, data, fmt);
            }
            else {
                result = std::to_chars(str_beg, buff_end, data, fmt, format.precision);
            }

            if (result.ec == std::errc{}) {
                this->set_output_ptr_unchecked(
                    safe_reinterpret_cast<unsigned char*>(result.ptr));
            }
            else {
                this->set_error(__FILE__, __LINE__, error_code::no_space_in_buffer);
                return;
            }
        }

        //char char8
        template<typename T, std::enable_if_t<
            is_char_type_v<T>
            && sizeof(T) == 1
            >* = nullptr>
        void encode(T data, [[maybe_unused]] format_type format) noexcept {
            if (this->output_has_space()) {
                auto str_begin{ this->output_ptr() };
                *str_begin = *safe_reinterpret_cast<const unsigned char*>(&data);
                this->advance_output_unchecked(1);
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
        void encode(byte_span<T> data, [[maybe_unused]] format_type format) noexcept {
			FSTLOG_ASSERT(data.data() != nullptr);
			static_assert(std::numeric_limits<unsigned char>::digits == 8);
            constexpr int bit_size{ sizeof(T) * std::numeric_limits<unsigned char>::digits };
            if constexpr (bit_size == 8) {
                auto buff_ptr{ this->output_ptr() };
                const auto bytes{ data.size_bytes() };
                if (static_cast<std::size_t>(this->output_end() - buff_ptr) >= bytes) {
                    memcpy(buff_ptr, data.data(), bytes);
                    this->advance_output_unchecked(bytes);
                }
                else {
                    this->set_error(__FILE__, __LINE__, error_code::no_space_in_buffer);
                }
            }
            else {
                const unsigned char* str_begin{ data.data() };
                std::size_t char_num{ (std::numeric_limits<std::size_t>::max)() };
                const auto result = detail::utf8conv<bit_size, T, unsigned char>(
                    str_begin,
                    str_begin + data.size_bytes(),
                    this->output_ptr(),
                    this->output_end(),
                    char_num);
                if (result.ec == error_code::none) {
                    this->set_output_ptr_unchecked(result.ptr);
                }
                else {
                    this->set_error(__FILE__, __LINE__, result.ec);
                }
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
            encode(safe_reinterpret_cast<std::uintptr_t>(data), format);
        }

        //bool
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
                if (format.type == 0
                    || format.type == 's')
                {
                    format.type = 'x';
                }
                encode(data.hash_, format);
            }
        }

        //nanosec_epoch
        template<typename T, std::enable_if_t<
            std::is_same_v<rm_cvref_t<T>, stamp_type>
            >* = nullptr>
        void encode(T data, [[maybe_unused]] format_type format) noexcept {
            const auto result = timestamp_to_chars(
                data,
				this->output_ptr(),
				this->output_end()
            );
            if (result.ec == error_code::none) {
                this->set_output_ptr_unchecked(result.ptr);
            }
            else {
                this->set_error(__FILE__, __LINE__, result.ec);
            }
        }

        static void reencode_tail_string(
            [[maybe_unused]] unsigned char* str_begin, 
            [[maybe_unused]] format_type format) noexcept {}
    };
}
