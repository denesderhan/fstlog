//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstring>
#include <format>
#include <limits>
#include <ranges>
#include <string_view>
#include <type_traits>
#pragma intrinsic(memcpy)

#include <fstlog/detail/rm_cvref_t.hpp>
#include <detail/byte_span.hpp>
#include <detail/checked_iterator.hpp>
#include <detail/utf_conv.hpp>
#include <detail/error_code.hpp>
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
#include <fstlog/detail/ut_cast.hpp>
#include <formatter/impl/detail/time_to_str_converter.hpp>

#ifdef FSTLOG_NOEXCEPTIONS
#error encoder_stdformat_mixin must use exceptions, but exceptions are disabled!
#endif

namespace fstlog {
	template<typename L>
    class encoder_stdformat_mixin : public L,
        public time_to_str_converter
    {
    public:
        typedef std::string_view format_type;
        using allocator_type = typename L::allocator_type;

        encoder_stdformat_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(encoder_stdformat_mixin(allocator_type{})))
			: encoder_stdformat_mixin(allocator_type{}) {}
		explicit encoder_stdformat_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
            : L(allocator) {}

		encoder_stdformat_mixin(const encoder_stdformat_mixin& other) noexcept(
			noexcept(encoder_stdformat_mixin::get_allocator())
			&& noexcept(encoder_stdformat_mixin(encoder_stdformat_mixin{}, allocator_type{})))
            : encoder_stdformat_mixin(other, other.get_allocator()) {}
		encoder_stdformat_mixin(const encoder_stdformat_mixin& other, allocator_type const& allocator) noexcept(
			noexcept(L(encoder_stdformat_mixin{}, allocator_type{}))
			&& noexcept(time_to_str_converter(encoder_stdformat_mixin{})))
            : L(other, allocator),
              time_to_str_converter(other) {}

        encoder_stdformat_mixin(encoder_stdformat_mixin&& other) = delete;
        encoder_stdformat_mixin& operator=(const encoder_stdformat_mixin& rhs) = delete;
        encoder_stdformat_mixin& operator=(encoder_stdformat_mixin&& rhs) = delete;
        
        ~encoder_stdformat_mixin() = default;
        
        //char, bool, string_view, void*, integral, float
        template<typename T, std::enable_if_t<
            std::is_same_v<rm_cvref_t<T>, char>
            || std::is_same_v<rm_cvref_t<T>, bool>
            || std::is_same_v<rm_cvref_t<T>, std::string_view>
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
                this->set_error(__FILE__, __LINE__, error_code::external_code_error);
            }
        }

        //char8
        template<typename T, std::enable_if_t<
            is_char_type_v<T>
            && sizeof(T) == 1
            && !std::is_same_v<rm_cvref_t<T>, char>
            >* = nullptr>
        void encode(T data, format_type format) noexcept {
            char c = *safe_reinterpret_cast<const char*>(&data);
            encode(c, format);
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
            || std::is_same_v<const T, const unsigned char>
        >* = nullptr>
        void encode(byte_span<T> data, format_type format) noexcept {
			FSTLOG_ASSERT(data.data() != nullptr);
			std::string_view str;
            if constexpr (sizeof(T) == 1) {
                str = std::string_view { 
                    safe_reinterpret_cast<const char*>(data.data()),
                    data.size_bytes() };
            }
            else {
                auto out_begin = safe_reinterpret_cast<unsigned char*>(&encoder_fmt_buffer_[0]);
                auto out_end = out_begin + encoder_fmt_buffer_.size();
                static_assert(std::numeric_limits<unsigned char>::digits == 8);
                constexpr int bit_size{ sizeof(T) * std::numeric_limits<unsigned char>::digits };
                std::size_t char_num{ (std::numeric_limits<std::size_t>::max)() };
				const unsigned char* in_ptr{ data.data() };
				const auto result = detail::utf8conv<bit_size, T, unsigned char>(
					in_ptr,
                    in_ptr + data.size_bytes(),
                    out_begin,
                    out_end,
                    char_num);
                if (result.ec != error_code::none) {
                    this->set_error(__FILE__, __LINE__, result.ec);
                    return;
                }

                std::size_t str_byte_size = static_cast<std::size_t>(result.ptr - out_begin);
                str = std::string_view { &encoder_fmt_buffer_[0], str_byte_size };
            }
            encode(str, format);
        }

        template<typename T, std::enable_if_t<
            !std::is_same_v<rm_cvref_t<T>, char>
        >* = nullptr>
        void encode(std::basic_string_view<T> strv, format_type format) noexcept {
			encode(byte_span<const T>{ strv.data(), strv.size() }, format);
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

        //nanosec_epoch
        template<typename T, std::enable_if_t<
            std::is_same_v<rm_cvref_t<T>, stamp_type>
            >* = nullptr>
        void encode(T data, format_type format) noexcept {
            auto out_begin = safe_reinterpret_cast<unsigned char*>(&encoder_fmt_buffer_[0]);
            auto out_end = out_begin + encoder_fmt_buffer_.size();
            auto result = timestamp_to_chars(data, out_begin, out_end);
            if (result.ec == error_code::none) {
                encode(
                    std::string_view{ 
						&encoder_fmt_buffer_[0],
						static_cast<std::size_t>(result.ptr - out_begin) },
                    format);
            }
            else {
                this->set_error(__FILE__, __LINE__, result.ec);
            }
        }

		void reencode_tail_string(unsigned char* str_begin, format_type format) noexcept {
			FSTLOG_ASSERT(
				str_begin >= this->output_begin() 
				&& str_begin <= this->output_ptr());
			const std::size_t str_byte_size = 
				static_cast<std::size_t>(this->output_ptr() - str_begin);
			if (str_byte_size <= encoder_fmt_buffer_.size()) {
				memcpy(
					&encoder_fmt_buffer_[0],
					str_begin,
					str_byte_size);
				this->set_output_ptr_unchecked(str_begin);
				encode(std::string_view{ &encoder_fmt_buffer_[0], str_byte_size }, format);
			}
		}

        private:
            std::array<char, 2048> encoder_fmt_buffer_{ 0 };
    };
}
