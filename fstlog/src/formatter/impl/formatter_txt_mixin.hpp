//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstddef>
#include <string_view>
#include <limits>

#include <detail/byte_span.hpp>
#include <detail/error.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <detail/utf8_helper.hpp>
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
			while (true) {
				const auto text_beg = out_pos;
				auto error = parse_fmt_text(in_pos, in_end, out_pos, out_end);
				if (error != error_code::none) return error;
				error = sanitize_utf8_str(text_beg, out_pos);
				if (error != error_code::none) return error;
				if (in_pos == in_end) break;
				buff_span_const field_name;
				buff_span_const format_spec;
				error = parse_fmt_repl_field(in_pos, in_end, field_name, format_spec);
				if (error != error_code::none) return error;
				const auto field_id = get_repl_field_id(field_name);
				if (field_id == logfield::Invalid) return error_code::fmt_bad;
				if (out_pos == out_end) return error_code::buff_full;
				*out_pos++ = ut_cast(field_id);
				
				if (field_id == logfield::Timestamp) {
					buff_span_const time_format_spec;
					error = time_format(format_spec, time_format_spec);
					if (error != error_code::none) return error;
					error = this->init_time_to_str_converter(time_format_spec);
					if (error != error_code::none) return error;
				}
				this->set_format(field_id, format_spec);
			}
			
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
			if (this->has_error()) write_error();

			const auto msg_begin{ out.data() };
			auto msg_end{ this->output_ptr() };
			FSTLOG_ASSERT(msg_end < msg_begin + out.size_bytes());
			*msg_end++ = '\n';
			return { msg_begin,
				static_cast<std::size_t>(msg_end - msg_begin) };
        }

    private:
		
		void encode_error(error err) noexcept {
			static constexpr std::string_view msg{ "[Log error!: " };
			const std::string_view err_msg{ err.message(), std::strlen(err.message()) };
			const std::string_view file{ err.file(), std::strlen(err.file()) };
			const auto format = this->get_default_format();
			this->encode(msg, format);
			this->encode(err_msg, format);
			this->encode(' ', format);
			this->encode(file, format);
			this->encode(':', format);
			this->encode(err.line(), format);
		}

		void write_error() noexcept {
			const auto err = this->get_error();
			this->clear_error();
			const auto format = this->get_default_format();
#ifndef NDEBUG
			// try to append error message to log line
			this->encode(' ', format);
			encode_error(err);
			if (this->has_error()) {
				//if fails, delete log line and format error message
				this->clear_error();
				this->output_span_init({ 
					this->output_begin(),
					static_cast<std::size_t>(this->output_end() - this->output_begin()) });
				encode_error(err);
			}
			//if fails write short error
			if (this->has_error()) {
				this->clear_error();
				this->output_span_init({
					this->output_begin(),
					static_cast<std::size_t>(this->output_end() - this->output_begin()) });
				this->encode(std::string_view{"Log error!:"}, format);
				this->encode(ut_cast(err.code()), format);
			}
#else
			this->encode(std::string_view{ " Log error!:" }, format);
			this->encode(ut_cast(err.code()), format);
#endif // !NDEBUG
			//setting back original error
			this->set_error(err.file(), err.line(), err.code());
		}

        void write_field(logfield field_id) noexcept {
			FSTLOG_ASSERT(ut_cast(field_id) > 0 && ut_cast(field_id) < ut_cast(logfield::Args));
			if (field_id == logfield::Severity) {
                this->encode(
                    severity_txt(this->severity()), 
                    this->get_format(logfield::Severity));
            }
            else if (field_id == logfield::Timestamp) {
                this->encode(
                    this->timestamp(),
                    this->get_format(logfield::Timestamp));
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
				sanitize_utf8_str(str_begin, this->output_ptr());
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
			const auto fb{ log_format_string() };
			auto ch = fb.data();
			const auto ch_end = ch + fb.size_bytes();
            auto pos = this->output_ptr();
            const auto end = this->output_end();
			
			while (!this->has_error() && ch < ch_end) {
                if ((*ch & 0b11100000) != 0) {
                    if (pos < end) *pos++ = *ch;
					else  this->set_error(__FILE__, __LINE__, error_code::buff_full);
                }
                else if(*ch < ut_cast(logfield::Args) ){
                    this->set_output_ptr_unchecked(pos);
                    write_field(logfield(*ch));
                    pos = this->output_ptr();
                }
                else {
                    this->set_error(__FILE__, __LINE__, error_code::input_bad);
                }
                ch++;
            }
				this->set_output_ptr_unchecked(pos);
			}
	
        void write_message_field() noexcept {
			const unsigned char* in_pos = message_fmt_string_.data();
			const unsigned char* const in_end = in_pos + message_fmt_string_.size_bytes();
			const unsigned char* const out_end = this->output_end();	
			bool has_repl_field = true;
			bool has_input = this->seek_field(logfield::Args);
			auto error = error_code::none;
            do {
				auto out_pos = this->output_ptr();
				auto form = this->get_default_format();
				error = parse_fmt_text(in_pos, in_end, out_pos, out_end);
				if (error != error_code::none) break;
				if (in_pos < in_end) {
					buff_span_const field_name;
					buff_span_const format_spec;
					error = parse_fmt_repl_field(in_pos, in_end, field_name, format_spec);
					if (error != error_code::none) break;
					form = this->get_format(format_spec);
				}
                else {
                    has_repl_field = false;
                    if (has_input && out_pos < out_end) {
						*out_pos++ = ' ';
                    }
                }
				this->set_output_ptr_unchecked(out_pos);

                if (has_input) {
                    auto type_signature = L::get_signature_skip_arg_header();
					if (this->has_error()) break;
                    if (type_signature.size_bytes() <= 1) {
                        L::process_element(type_signature, form);
                    }
                    //aggregate
                    else {
                        auto str_begin = this->output_ptr();
                        L::process_element(type_signature, this->get_default_format());
						this->reencode_tail_string(str_begin, form);
                    }
                    has_input = !this->end_of_input_data();
                }
                else if( has_repl_field ) {
					this->encode(std::string_view{ "{missing}" }, this->get_default_format());
                }
            } while (!this->has_error()
                && (has_repl_field || has_input));
			if(error != error_code::none) this->set_error(__FILE__, __LINE__, error);
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
