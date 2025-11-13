//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <format>
#include <string_view>
#include <type_traits>

#include <fstlog/detail/rm_cvref_t.hpp>
#include <detail/unaligned_span.hpp>
#include <detail/checked_iterator.hpp>
#include <detail/utf_conv.hpp>
#include <fstlog/detail/error_code.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <fstlog/detail/convert_to_basic_string_view.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/is_char_type.hpp>
#include <fstlog/detail/is_string_like.hpp>
#include <fstlog/detail/log_type_metadata.hpp>
#include <fstlog/detail/types.hpp>
#include <fstlog/detail/noexceptions.hpp>
#include <fstlog/detail/str_hash_fnv.hpp>

#ifdef FSTLOG_NOEXCEPTIONS
#error encoder_stdformat_mixin must use exceptions, but exceptions are disabled!
#endif

namespace fstlog {
    template<typename L>
    class encoder_stdformat_mixin : public L {
    public:
        using format_type = std::string_view;
        
        //bool, void*, integral, float
        template<typename T, std::enable_if_t<
            std::is_same_v<rm_cvref_t<T>, bool>
            || std::is_same_v<rm_cvref_t<T>, void*>
            || std::is_same_v<rm_cvref_t<T>, const void*>
            || std::is_same_v<rm_cvref_t<T>, const volatile void*>
            || (std::is_integral_v<T> && !is_char_type_v<T>)
            || std::is_floating_point_v<T>
            >* = nullptr>
        void encode(T data, format_type format) noexcept {
            try {
                detail::checked_iterator it(
                    safe_reinterpret_cast<char*>(this->output_ptr()),
                    safe_reinterpret_cast<char*>(this->output_end()));
                auto result = std::vformat_to(
                    it,
                    format,
                    std::make_format_args(data));
                this->set_output_ptr_unchecked(
                    safe_reinterpret_cast<unsigned char*>(result.get_ptr()));
            }
            catch(...) {
                this->set_error(__FILE__, __LINE__, error_code::extern_err);
            }
        }

        // char
        void encode(char data, format_type format) noexcept {
            auto form_str = small_string<24>(format);
            const unsigned char byte = *safe_reinterpret_cast<const unsigned char*>(&data);
            FSTLOG_ASSERT(format.size() >= 2);
            const char form_type = *(form_str.data() + form_str.size() - 2);
            const char* type_ptr = "xXdbBo";
            while (*type_ptr != 0 && *type_ptr != form_type) type_ptr++;
            // safe ASCII and formatted textually
            if (*type_ptr == 0
                && byte < 0x80 && detail::utf::safe_utf_code_point(byte))
            {
                // remove 'c' (we encode as a string)
                // 'c' would cause problem
                if (form_type == 'c') {
                    form_str = this->get_format_align_width(form_str);
                }
                encode(byte_span_const{ &byte, 1 }, form_str);
            }
            // invalid, unsafe or formatted numerically
            else {
                // replace/add form_type (we encode as a number)
                // 'c' would cause problem
                if (*type_ptr == 0) {
                    form_str = this->get_format_align_width(form_str, 'x');
                }
                encode(byte, form_str);
            }
        }

        // char16, char32
        template<typename T, std::enable_if_t<
            std::is_same_v<T, char16_t>
            || std::is_same_v<T, char32_t>
        >* = nullptr>
        void encode(T data, format_type format) noexcept {
            auto form_str = small_string<24>(format);
            FSTLOG_ASSERT(format.size() >= 2);
            const char form_type = *(form_str.data() + form_str.size() - 2);
            const char* type_ptr = "xXdbBo";
            while (*type_ptr != 0 && *type_ptr != form_type) type_ptr++;
            // safe ASCII and formatted textually
            if (*type_ptr == 0
                && detail::utf::safe_utf_code_point(data))
            {
                if (form_type == 'c') {
                    form_str = this->get_format_align_width(form_str);
                }
                encode(unaligned_span<const T>{ &data, 1 }, form_str);
            }
            else {
                if (*type_ptr == 0) {
                    form_str = this->get_format_align_width(form_str, 'x');
                }
                encode(static_cast<std::uintmax_t>(data), form_str);
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

        //string in buffer
        template<typename T, std::enable_if_t<
            std::is_same_v<std::remove_const_t<T>, unsigned char>
            || (is_char_type_v<T> &&
                !std::is_same_v<std::remove_const_t<T>, char>)
        >* = nullptr>
        void encode(unaligned_span<const T> data, format_type format) noexcept {
            if (data.empty()) return;
            constexpr int bit_size{ sizeof(T) * 8 };
            unaligned_span output(
                safe_reinterpret_cast<unsigned char*>(encoder_fmt_buffer_.data()),
                encoder_fmt_buffer_.size());
            const auto result = detail::utf::utf8conv<bit_size>(data, output);
            if (result.ec != error_code::none) {
                this->set_error(__FILE__, __LINE__, result.ec);
            }

            std::size_t str_byte_size = encoder_fmt_buffer_.size() - output.size();
            std::string_view str = std::string_view { encoder_fmt_buffer_.data(), str_byte_size};
            try {
                detail::checked_iterator it(
                    safe_reinterpret_cast<char*>(this->output_ptr()),
                    safe_reinterpret_cast<char*>(this->output_end()));
                auto result_fmt = std::vformat_to(
                    it,
                    format,
                    std::make_format_args(str));
                this->set_output_ptr_unchecked(
                    safe_reinterpret_cast<unsigned char*>(result_fmt.get_ptr()));
            }
            catch (...) {
                this->set_error(__FILE__, __LINE__, error_code::extern_err);
            }
        }

        //pointer
        template<typename T, std::enable_if_t<
            std::is_pointer_v<std::remove_reference_t<T>>
            && !std::is_same_v<rm_cvref_t<std::remove_pointer_t<rm_cvref_t<T>>>, void>
            >* = nullptr>
        void encode(T data, format_type format) noexcept {
            encode<const void*>(data, format);
        }
        
        //str_hash_fnv
        template<typename T, std::enable_if_t<
            std::is_same_v<rm_cvref_t<T>, str_hash_fnv>
            >* = nullptr>
        void encode(T data, format_type format) noexcept {
            auto str = this->convert_hash(data);
            if (!str.empty()) {
                encode(str, format);
            }
            else {
                encode(data.hash_, "{:#x}");
            }
        }

        void reencode_tail_string(unsigned char* str_begin, format_type format) noexcept {
            FSTLOG_ASSERT(str_begin >= this->output_begin()    
                && str_begin <= this->output_ptr());
            if (format == "{}" || format == "{:}") return;
            const std::size_t str_byte_size = 
                static_cast<std::size_t>(this->output_ptr() - str_begin);
            if (str_byte_size <= encoder_fmt_buffer_.size()) {
                std::memcpy(&encoder_fmt_buffer_[0], str_begin, str_byte_size);
                this->set_output_ptr_unchecked(str_begin);
                encode(std::string_view{ &encoder_fmt_buffer_[0], str_byte_size }, format);
            }
        }

        private:
            std::array<char, 2048> encoder_fmt_buffer_{ 0 };
    };
}
