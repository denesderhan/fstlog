//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <limits>
#include <type_traits>
#include <cstring>
#pragma intrinsic(memcpy)

#include <detail/error_code.hpp>
#include <detail/byte_span.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/internal_arg_header.hpp>
#include <fstlog/detail/internal_msg_header.hpp>
#include <fstlog/detail/log_element_type.hpp>
#include <fstlog/detail/log_type.hpp>
#include <fstlog/detail/padded_size.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
    template<typename L>
    class decoder_internal_mixin : public L {
    public:
        using allocator_type = typename L::allocator_type;

        decoder_internal_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(decoder_internal_mixin(allocator_type{})))
			: decoder_internal_mixin(allocator_type{}) {}
        explicit decoder_internal_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
			: L(allocator) {}

		decoder_internal_mixin(const decoder_internal_mixin& other) noexcept(
			noexcept(decoder_internal_mixin::get_allocator())
			&& noexcept(decoder_internal_mixin(decoder_internal_mixin{}, allocator_type{})))
            : decoder_internal_mixin(other, other.get_allocator()) {}
		decoder_internal_mixin(const decoder_internal_mixin& other, allocator_type const& allocator) noexcept(
			noexcept(L(decoder_internal_mixin{}, allocator_type{})))
            : L(other, allocator) {}

        decoder_internal_mixin(decoder_internal_mixin&& other) = delete;
        decoder_internal_mixin& operator=(const decoder_internal_mixin& rhs) = delete;
        decoder_internal_mixin& operator=(decoder_internal_mixin&& rhs) = delete;
        
        ~decoder_internal_mixin() = default;

        void decoder_set_input(buff_span_const msg) noexcept {
			this->input_span_init(msg);
            this->advance_input(internal_msg_header::padded_data_size);
			if (!this->has_error()) {
				this->set_header(msg.data());
				if (this->message_type() != log_msg_type::Internal
					|| this->message_size() != msg.size_bytes())
				{
					this->set_error(__FILE__, __LINE__, error_code::input_contract_violation);
				}
			}
        }

		buff_span_const get_signature_skip_arg_header() noexcept {
			const auto header_begin{ this->input_ptr() };
			internal_arg_header<char> min_size_header;
			constexpr std::size_t signature_offset = 
				sizeof(min_size_header.arg_size) 
				+ sizeof(min_size_header.signature_length);
			const auto signature_begin = header_begin + signature_offset;
			
			const auto end_ptr{ this->input_end() };
			if (signature_begin < end_ptr) {
				memcpy(
					&min_size_header,
					header_begin,
					internal_arg_header<char>::data_size);
				if (min_size_header.signature_length != 0) {
					const auto padded_header_size =
						padded_size<constants::internal_msg_data_alignment>(
							signature_offset + min_size_header.signature_length);
					this->advance_input(padded_header_size);
					if (!this->has_error()) {
						return { signature_begin, min_size_header.signature_length };
					}
					else return { header_begin, 0 };
				}
			}
			this->set_error(__FILE__, __LINE__, error_code::input_contract_violation);
			return { header_begin, 0 };
        }

        void skip_argument() noexcept {
			const auto arg_begin{ this->input_ptr() };
			if (static_cast<std::size_t>(this->input_end() - arg_begin)
				>= internal_arg_header<char>::data_size)
			{
				internal_arg_header<char> header;
				memcpy(
					&header,
					arg_begin,
					internal_arg_header<char>::data_size);
				this->advance_input(header.arg_size);
			}
			else this->set_error(__FILE__, __LINE__, error_code::input_contract_violation);
        }

        template<typename T, bool padded = true>
        void get_data(T& out) noexcept {
            static_assert(log_type_v<T> != log_element_type::Unloggable
                && log_type_v<T> != log_element_type::Aggregate
                && log_type_v<T> != log_element_type::String, "Only simple type!");
            const auto data_ptr = this->input_ptr();
            skip_data<T, padded>();
            if (!this->has_error()) {
                memcpy(
                    &out,
                    data_ptr,
                    sizeof(T));
            }
        }

		template <typename T>
        void get_data(byte_span<const T>& out) noexcept {
            static_assert(
                (std::numeric_limits<msg_counter>::max)()
                <= (std::numeric_limits<std::size_t>::max)() / sizeof(T), "String size counter too big!");
            msg_counter str_size{ 0 };
            get_data(str_size);
            if (!this->has_error()) {
                const auto str_begin = this->input_ptr();
                const auto byte_size = static_cast<std::size_t>(str_size) * sizeof(T);
                this->advance_input(padded_size<constants::internal_msg_data_alignment>(byte_size));
                if (!this->has_error()) {
					out = byte_span<const T>{ str_begin, byte_size };
                }
            }
        }

        template<typename T, bool padded = true>
        void skip_data() noexcept {
            static_assert(log_type_v<T> != log_element_type::Aggregate &&
                log_type_v<T> != log_element_type::String, "Can only skip simple type!");
            static_assert(padded_size<constants::internal_msg_data_alignment>(sizeof(T)) <=
                (std::numeric_limits<std::uint32_t>::max)());
            if constexpr (padded) {
                this->advance_input(padded_size<constants::internal_msg_data_alignment>(
                    static_cast<std::uint32_t>(sizeof(T))));
            }
            else {
                this->advance_input(static_cast<std::uint32_t>(sizeof(T)));
            }
        }
    };
}
