//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstddef>
#include <cstring>
#include <limits>
#include <string_view>
#pragma intrinsic(memcpy)

#include <detail/byte_span.hpp>
#include <detail/error.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <formatter/impl/detail/format_str_helper.hpp>
#include <formatter/impl/detail/logfield.hpp>
#include <formatter/impl/detail/policy_txt.hpp>
#include <formatter/impl/detail/severity_txt.hpp>
#include <formatter/impl/detail/tz_format.hpp>
#include <formatter/impl/detail/valid_strftime_string.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/padded_size.hpp>
#include <fstlog/detail/ut_cast.hpp>

namespace fstlog {
    template<typename L>
    class formatter_txt_mixin : public L
    {
    public:
        using allocator_type = typename L::allocator_type;

        formatter_txt_mixin() noexcept(
			noexcept(allocator_type{})
			&& noexcept(L(allocator_type{})))
			: formatter_txt_mixin(allocator_type{}) {}
        explicit formatter_txt_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
            : L(allocator) {}

		formatter_txt_mixin(const formatter_txt_mixin& other) noexcept(
			noexcept(this->get_allocator())
			&& noexcept(L(formatter_txt_mixin{}, allocator_type{})))
            : formatter_txt_mixin(other, other.get_allocator()) {}
        formatter_txt_mixin(const formatter_txt_mixin& other, allocator_type const& allocator) noexcept(
			noexcept(L(formatter_txt_mixin{}, allocator_type{})))
            : L(other, allocator),
            formatting_buffer_ { other.formatting_buffer_ }, //noexcept
            log_fmt_str_len_{ other.log_fmt_str_len_ }, //noexcept
			msg_fmt_str_start_{ other.msg_fmt_str_start_ }, //noexcept
			msg_fmt_str_cap_{ other.msg_fmt_str_cap_ } { //noexcept
        }

        formatter_txt_mixin(formatter_txt_mixin&& other) = delete;
        formatter_txt_mixin& operator=(const formatter_txt_mixin& rhs) = delete;
        formatter_txt_mixin& operator=(formatter_txt_mixin&& rhs) = delete;
        
        ~formatter_txt_mixin() = default;

		error_code formatter_init(buff_span_const format_string) noexcept {
			if (format_string.empty()) {
				format_string = buff_span_const {
					safe_reinterpret_cast<const unsigned char*>(config::default_format_string.data()), 
					config::default_format_string.size() };
			}
			return parse_format_string(format_string);
		}

		inline error_code parse_format_string(buff_span_const format_string) {
			auto in_pos = format_string.data();
			const auto in_end = in_pos + format_string.size_bytes();
			auto out_pos = formatting_buffer_.data();
			const auto out_end = out_pos + formatting_buffer_.size();
			bool has_message_field = false;
			while (true) {
				auto error = parse_fmt_text(in_pos, in_end, out_pos, out_end);
				if (error != error_code::none) return error;
				if (in_pos == in_end) break;
				buff_span_const field_name;
				buff_span_const format_spec;
				error = parse_fmt_repl_field(in_pos, in_end, field_name, format_spec);
				if (error != error_code::none) return error;
				const auto field_id = get_repl_field_id(field_name);
				if (field_id == logfield::Invalid) return error_code::fmt_bad;
				else if (field_id == logfield::Message) has_message_field = true;
				if (out_pos == out_end) return error_code::buff_full;
				*out_pos++ = ut_cast(field_id);

				if (field_id == logfield::Timestamp) {
					error = this->init_encoder_timestamp(format_spec);
					if (error != error_code::none) return error;
				}
				else {
					if (!valid_format_spec(format_spec)) return error_code::fmt_bad;
					this->set_format(field_id, format_spec);
				}
			}
			
			if (!has_message_field) return error_code::fmt_bad;

			log_fmt_str_len_ =
				static_cast<std::uint32_t>(out_pos - formatting_buffer_.data());
			msg_fmt_str_start_ =
				padded_size<constants::cache_ls_nosharing>(log_fmt_str_len_);
			msg_fmt_str_cap_ =
				static_cast<std::uint32_t>(formatting_buffer_.size() - msg_fmt_str_start_);
			if (msg_fmt_str_cap_ < formatting_buffer_.size() / 4) return error_code::str_long;
			return error_code::none;
		}

        buff_span format_message(
            buff_span_const in,
            buff_span out) noexcept
        {
			//asserting init is called prior to
			FSTLOG_ASSERT(msg_fmt_str_cap_ != 0);
			FSTLOG_ASSERT(in.data() != nullptr);
			FSTLOG_ASSERT(in.size_bytes() <= 
				(std::numeric_limits<msg_counter>::max)());
			FSTLOG_ASSERT(out.data() != nullptr);
			// we have to always have space for '\n' 
			// and short error message.
			FSTLOG_ASSERT(out.size_bytes() >= 64);

			this->clear_error();
			this->decoder_set_input(in);
			if (!this->has_error()) this->init_logfields();
			if (!this->has_error()) set_message_format_string();
			// set_message_format_string() can use this->output!!
			// we have to init output after calling set_message_format_string()
			// we initialize it 1 byte less, leaving space for '\n'
			this->output_span_init(
				{ out.data(),
				static_cast<std::size_t>(out.size_bytes() - 1) });
			if (!this->has_error()) write_log_line();
			const auto msg_begin{ out.data() };
			auto msg_end{ this->output_ptr() };

			if (this->has_error()) {
				msg_end = this->get_error().write_to(msg_end, this->output_end());
			}

			FSTLOG_ASSERT(msg_end < msg_begin + out.size_bytes());
			*msg_end++ = '\n';
			return { msg_begin,
				static_cast<std::size_t>(msg_end - msg_begin) };
		}

	private:

		void write_field(logfield field_id) noexcept {
			FSTLOG_ASSERT(ut_cast(field_id) > 0 && ut_cast(field_id) < ut_cast(logfield::Args));
			if (field_id == logfield::Severity) {
				this->encode(
					severity_txt(this->severity()),
					this->get_format(logfield::Severity));
			}
			else if (field_id == logfield::Timestamp) {
				this->encode_timestamp(	this->timestamp());
			}
			else if (field_id == logfield::Channel) {
				this->encode(
					this->channel(),
					this->get_format(logfield::Channel));
			}
			else if (field_id == logfield::Policy) {
				this->encode(
					policy_txt(this->policy()),
					this->get_format(logfield::Policy));
			}
			else if (field_id == logfield::Message) {
				const auto str_begin = this->output_ptr();
				write_message_field();
				this->reencode_tail_string(
					str_begin,
					this->get_format(logfield::Message));
			}
			else {
				if (this->seek_field(field_id)) {
					auto type_signature = L::get_signature_skip_arg_header();
					if (!this->has_error())
						L::process_element(type_signature, this->get_format(field_id));
				}
				else {
					this->encode(get_repl_field_name(field_id),
						this->get_format_align_width(this->get_format(field_id)));
				}
			}
		}

		void write_log_line() noexcept {
			const auto str{ log_format_string() };
			auto str_pos = str.data();
			const auto str_end = str_pos + str.size_bytes();
			while (str_pos < str_end) {
				// if *str_pos <= 0x1F (0b0001'1111) then it is an encoded logfield id, all else is a char
				if ((*str_pos & 0b1110'0000) == 0) {
					write_field(logfield(*str_pos++));
					if (this->has_error()) return;
				}
				else {
					if (this->output_has_space()) {
						*this->output_ptr() = *str_pos++;
						this->advance_output_unchecked(1);
					}
					else {
						this->set_error(__FILE__, __LINE__, error_code::buff_full);
						return;
					}
				}
			}
		}

		void write_message_field() noexcept {
			const unsigned char* in_pos = message_fmt_string_.data();
			const unsigned char* const in_end = in_pos + message_fmt_string_.size_bytes();
			write_message_text(in_pos, in_end);
			if (this->has_error()) return;
			bool has_data = this->seek_field(logfield::Args);
			// message is only a simple text
			if ((!has_data && in_pos == in_end)) return;
			// no data with replacement field or data without replacement field
			if (!has_data || in_pos == in_end) {
				this->set_error(__FILE__, __LINE__, error_code::fmt_bad);
				return;
			}
			while (in_pos < in_end) {
				buff_span_const field_name;
				buff_span_const format_spec;
				auto error = parse_fmt_repl_field(in_pos, in_end, field_name, format_spec);
				if (error != error_code::none) {
					this->set_error(__FILE__, __LINE__, error);
					return;
				}
				write_message_data(format_spec);
				if (this->has_error()) return;
				
				write_message_text(in_pos, in_end);
				if (this->has_error()) return;
			}
			// more data than replacement field
			if (!this->end_of_input_data()) {
				this->set_error(__FILE__, __LINE__, error_code::fmt_bad);
			}
		}

		void write_message_text(const unsigned char* &in_pos, const unsigned char* in_end) {
			auto out_pos = this->output_ptr();
			auto error = parse_fmt_text(in_pos, in_end, out_pos, this->output_end());
			this->set_output_ptr_unchecked(out_pos);
			if (error != error_code::none) {
				this->set_error(__FILE__, __LINE__, error);
			}
		}

		void write_message_data(buff_span_const format_spec) {
			auto type_signature = L::get_signature_skip_arg_header();
			// no data
			if (this->has_error()) return;
#ifndef NDEBUG
			if (!valid_format_spec(format_spec)) {
				this->set_error(__FILE__, __LINE__, error_code::fmt_bad);
			}
#endif
			auto form = this->get_format(format_spec);
			// non aggregate
			if (type_signature.size_bytes() <= 1) {
				L::process_element(type_signature, form);
			}
			//aggregate
			else {
				auto str_begin = this->output_ptr();
				L::process_element(type_signature, this->get_default_format());
				this->reencode_tail_string(str_begin, form);
			}
		}
		
		void set_message_format_string() noexcept {
			// message is mandatory 
			// (error was raised in init_logfields() if not present!)
			this->seek_field(logfield::Message);
			auto type_signature = L::get_signature_skip_arg_header();
			if (this->has_error()) return;
			FSTLOG_ASSERT(!type_signature.empty());
						
			auto data_type = *type_signature.data();
			const unsigned char msg_type = data_type & log_element_type_bitmask;
			const unsigned char msg_meta = data_type & log_type_metadata_bitmask;
			if ( msg_type == ut_cast(log_element_type::String)
				&& (msg_meta == ut_cast(char_type::Char) || msg_meta == ut_cast(char_type::Char8)))
			{
				this->get_data(message_fmt_string_);
			}
			else {
				// if the message string has to be decoded/converted to utf8 string
				// we do this into the free space of the formatting buffer
				this->output_span_init(log_message_buffer());
				L::process_element(type_signature, this->get_default_format());
				message_fmt_string_ = {
					this->output_begin(),
					static_cast<std::size_t>(this->output_ptr() - this->output_begin()) };
			}
		}

		buff_span_const log_format_string() noexcept {
			return buff_span_const{ 
				formatting_buffer_.data(), log_fmt_str_len_ };
		}

		buff_span log_message_buffer() noexcept {
			return buff_span{
				formatting_buffer_.data() + msg_fmt_str_start_, 
				msg_fmt_str_cap_ };
		}

		alignas(constants::cache_ls_nosharing) std::array<unsigned char, 512> formatting_buffer_{ 0 };
		std::uint32_t log_fmt_str_len_{ static_cast<std::uint32_t>(formatting_buffer_.size()) };
		std::uint32_t msg_fmt_str_start_{ static_cast<std::uint32_t>(formatting_buffer_.size()) };
		std::uint32_t msg_fmt_str_cap_{ 0 };
		buff_span_const message_fmt_string_;
    };
}
