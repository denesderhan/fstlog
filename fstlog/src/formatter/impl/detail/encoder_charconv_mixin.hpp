//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <charconv>
#include <cstddef>
#include <limits>
#include <string_view>
#include <type_traits>

#include <detail/byte_span.hpp>
#include <fstlog/detail/error_code.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <detail/utf_conv.hpp>
#include <formatter/impl/detail/encoder_helper.hpp>
#include <formatter/impl/detail/format_setting_txt.hpp>
#include <formatter/impl/detail/shift_fill.hpp>
#include <formatter/impl/detail/time_to_str_converter.hpp>
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
			
			// set base
			const int base = detail::get_int_base(format.type);

			// write the prefix 
			// do not write prefix if data is octal 0 (write 0 not 00)
			if (format.alternate
				&& !(format.type == 'o' && data == 0))
			{
				detail::write_prefix(base, format.type, buffer_pos);
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
			if (format.type == 'X') {
				detail::to_upper_case(digits_start, result.ptr);
			}

			// apply alligning and filling with fill_char
			if (format.width != 0) {
				//setting default alignment for integrals
				if (format.align == 0) format.align = '>';
				const auto byte_size{ str_end - str_begin };
				auto sf_result = shift_fill(
					str_begin,
					buffer_end,
					byte_size,
					byte_size,
					format.width,
					format.align,
					format.fill_char);
				str_end = sf_result.ptr;
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
				detail::to_upper_case(num_begin, result.ptr);
            }
            
			unsigned char* str_end = safe_reinterpret_cast<unsigned char*>(result.ptr);
			
			// apply fill align if needed
            if (format.width != 0) {
                //setting default alignment for floats
                if (format.align == 0) format.align = '>';
                const auto byte_size{ str_end - str_begin };
                auto sf_result = shift_fill(
					str_begin,
                    buffer_end,
                    byte_size,
                    byte_size,
                    format.width,
                    format.align,
                    format.fill_char);
				str_end = sf_result.ptr;
            }
			// update buffer pointer to the first free byte
			this->set_output_ptr_unchecked(str_end);
        }

        // character
        template<typename T, std::enable_if_t<
            is_char_type_v<T>
            >* = nullptr>
        void encode(T data, format_type format) noexcept {
            encode(byte_span<T>{ &data, 1 }, format);
        }

        // string in buffer
        template<typename T, std::enable_if_t<
            is_char_type_v<T>
            || std::is_same_v<std::remove_const_t<T>, unsigned char>
            >* = nullptr>
        void encode(byte_span<T> data, format_type format) noexcept {
			FSTLOG_ASSERT(data.data() != nullptr);
            const auto out_begin{ this->output_ptr() };
            auto out_end{ this->output_end() };
            const unsigned char* str_begin{ data.data() };
            const auto data_byte_size = data.size_bytes();

            static_assert(std::numeric_limits<unsigned char>::digits == 8);
            constexpr int bit_size{ sizeof(T) * std::numeric_limits<unsigned char>::digits };
            std::size_t char_num{ format.precision };
            const auto result = detail::utf8conv<bit_size, T>(
                str_begin,
                str_begin + data_byte_size,
                out_begin,
                out_end,
                char_num);
            if (result.ec != error_code::none) {
                this->set_error(__FILE__, __LINE__, result.ec);
                return;
            }

            // fill align
			if (format.width != 0) {
				const auto byte_size = static_cast<std::size_t>(result.ptr - out_begin);
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
			// no fill align
            else {
                this->set_output_ptr_unchecked(result.ptr);
            }
        }

		// string_view
        template<typename T>
        void encode(std::basic_string_view<T> strv, format_type format) noexcept {
			encode(byte_span<const T>{ strv.data(), strv.size() }, format);
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

        // nanosec_epoch
        template<typename T, std::enable_if_t<
            std::is_same_v<rm_cvref_t<T>, stamp_type>
            >* = nullptr>
        void encode(T data, format_type format) noexcept {
            const auto str_begin{ this->output_ptr() };
            const auto buffer_end{ this->output_end() };
            // convert timestamp to string
			const auto result = timestamp_to_chars(data, str_begin, buffer_end);
            if (result.ec != error_code::none) {
				this->set_error(__FILE__, __LINE__, result.ec);
				return;
			}
			auto str_end = result.ptr;

			// fill align
            if (format.width != 0) {
				const auto [byte_size, char_num] = detail::utf8_str_trim(str_begin, str_end);
                if (char_num < format.width) { 
					auto sf_result = shift_fill(
						str_begin,
						buffer_end,
						byte_size,
						char_num,
						format.width,
						format.align,
						format.fill_char);
					str_end = sf_result.ptr;
				}
            }
			// update first free pos in buffer
            this->set_output_ptr_unchecked(str_end);
        }

        void reencode_tail_string(unsigned char* str_begin, format_type format) {
            unsigned char *str_end{ this->output_ptr() };
           	FSTLOG_ASSERT(str_begin >= this->output_begin() && str_begin <= str_end);
			const auto [str_byte_size, char_num] = detail::utf8_str_trim(str_begin, str_end, format.precision);
			str_end = str_begin + str_byte_size;
            FSTLOG_ASSERT(str_begin <= str_end);                        
            if (format.width > char_num) {
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
