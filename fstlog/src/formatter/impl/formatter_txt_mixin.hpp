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
#include <formatter/impl/detail/format_string_error.hpp>
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
            log_format_size_{ other.log_format_size_ }, //noexcept
			msg_template_start_{ other.msg_template_start_ }, //noexcept
			msg_template_capacity_{ other.msg_template_capacity_ } { //noexcept
        }

        formatter_txt_mixin(formatter_txt_mixin&& other) = delete;
        formatter_txt_mixin& operator=(const formatter_txt_mixin& rhs) = delete;
        formatter_txt_mixin& operator=(formatter_txt_mixin&& rhs) = delete;
        
        ~formatter_txt_mixin() = default;

		const char* formatter_init(buff_span_const format_string) noexcept {
			if (format_string.empty()) {
				format_string = buff_span_const {
					safe_reinterpret_cast<const unsigned char*>(config::default_format_string.data()), 
					config::default_format_string.size() };
			}
			const auto error = format_string_error(format_string);
			if (error != nullptr) return error;
			this->clear_error();
			this->output_span_init(formatting_buffer_);

			bool time_field_set{ false };
			while (true) {
				write_literal_escaped(format_string);
				if (this->has_error()) return "Format string too long!";
				if (format_string.empty()) break;
				auto replacement_field = get_replacement_field(format_string);
				const auto field_name = argument_name(replacement_field);
				const auto rf_id = get_repl_field_id(field_name);
				if (rf_id == logfield::Invalid) return "Invalid or empty field name!";
				if (!this->output_has_space()) return "Format string too long!";
				*this->output_ptr() = ut_cast(rf_id);
				this->advance_output_unchecked(1);
				auto form_spec = format_spec(replacement_field);

				if (rf_id == logfield::Timestamp) {
					if (time_field_set) return "Only one timestamp field allowed!";
					time_field_set = true;
					auto begin = form_spec.data();
					auto end = begin + form_spec.size_bytes();
					auto pos = skip_numbers(skip_align(begin, end), end);
					form_spec = buff_span_const{ begin, static_cast<std::size_t>(pos - begin) };
					auto precision = get_precision(pos, end);
					tz_format t_zone = get_zone(pos, end);

					auto time_fmt = buff_span_const{ pos, static_cast<std::size_t>(end - pos) };
					if (time_fmt.empty()) {
                        if (t_zone == tz_format::Local) {
							constexpr auto temp{ "%Y-%m-%d %H:%M:%S %z" };
							time_fmt = buff_span_const{
								safe_reinterpret_cast<const unsigned char*>(temp),
								sizeof("%Y-%m-%d %H:%M:%S %z") - 1 };
						}
						else {
							constexpr auto temp{ "%Y-%m-%d %H:%M:%S +0000" };
							time_fmt = buff_span_const{
								safe_reinterpret_cast<const unsigned char*>(temp),
								sizeof("%Y-%m-%d %H:%M:%S +0000") - 1};
						}
					}
					auto error2 = this->init_time_to_str_converter(time_fmt, precision, t_zone);
					if (error2 != nullptr) return error2;
				}
				this->set_format(rf_id, form_spec);
			}
			if (this->has_error()) {
				return this->get_error().message();
			}

			log_format_size_ =
				static_cast<std::uint32_t>(this->output_ptr() - formatting_buffer_.data());
			msg_template_start_ =
				padded_size<constants::cache_ls_nosharing>(log_format_size_);
			msg_template_capacity_ =
				static_cast<std::uint32_t>(formatting_buffer_.size() - msg_template_start_);
			if (msg_template_capacity_ < formatting_buffer_.size() / 4) return "Format string too long!";
			return nullptr;
		}

        buff_span format_message(
            buff_span_const in,
            buff_span out) noexcept
        {
			//asserting init is called prior to
			FSTLOG_ASSERT(msg_template_capacity_ != 0);
			FSTLOG_ASSERT(in.data() != nullptr);
			FSTLOG_ASSERT(in.size_bytes() <= 
				(std::numeric_limits<msg_counter>::max)());
			FSTLOG_ASSERT(out.data() != nullptr);
			//at least a '\n' must fit in the out buffer
			FSTLOG_ASSERT(out.size_bytes() >= 64 && "Out buffer too small!");
			this->clear_error();
			this->decoder_set_input(in);
			if (!this->has_error()) this->init_logfields();
			if (!this->has_error()) format_msg_template();
			this->output_span_init(
				{ out.data(),
				static_cast<std::size_t>(out.size_bytes() - 1) });
			if (!this->has_error()) write_fields();
			if (this->has_error()) format_error();

			const auto msg_begin{ out.data() };
            auto msg_end{ this->output_ptr() };
			sanitize_utf8_str(msg_begin, msg_end);
			*msg_end++ = '\n';
            return {msg_begin,
                static_cast<std::size_t>(msg_end - msg_begin)};
        }

    private:
		void format_msg_template() noexcept {
			msg_template_size_ = 0;
			this->output_span_init(
				{ formatting_buffer_.data() + msg_template_start_, 
				msg_template_capacity_ });
			// message is mandatory 
			// (error was raised in init_logfields() if not present!)
			this->seek_field(logfield::Message);
			auto type_signature = L::get_signature_skip_arg_header();
				if (this->has_error()) return;
			L::process_element(type_signature, this->get_default_format());
				if (this->has_error()) return;
			msg_template_size_ =
				static_cast<std::uint32_t>(this->output_ptr() - this->output_begin());
		}

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

		void format_error() noexcept {
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
                auto str_begin = this->output_ptr();
                process_message();
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

        void write_fields() noexcept {
			const auto fb{ log_format_str() };
			auto ch = fb.data();
			const auto ch_end = ch + fb.size_bytes();
            auto pos = this->output_ptr();
            const auto end = this->output_end();
			
			while (!this->has_error() && ch < ch_end) {
                if ((*ch & 0b11100000) != 0) {
                    if (pos < end) *pos++ = *ch;
					else  this->set_error(__FILE__, __LINE__, error_code::no_space_in_buffer);
                }
                else if(*ch < ut_cast(logfield::Args) ){
                    this->set_output_ptr_unchecked(pos);
                    write_field(logfield(*ch));
                    pos = this->output_ptr();
                }
                else {
                    this->set_error(__FILE__, __LINE__, error_code::input_contract_violation);
                }
                ch++;
            }
            this->set_output_ptr_unchecked(pos);
        }

        inline void write_literal_escaped(buff_span_const& str) noexcept {
			FSTLOG_ASSERT(str.data() != nullptr);
			auto str_pos = str.data();
            const auto str_end = str_pos + str.size_bytes();
            auto out_pos = this->output_ptr();
            const auto out_end = this->output_end();
            while (out_pos < out_end && str_pos < str_end) {
                const auto ch{ *str_pos };
                if (ch == '{' || ch == '}') {
                    if (str_pos < str_end
                        && *(str_pos + 1) == ch)
                    {
                        str_pos++;
                    }
                    else {
                        this->set_output_ptr_unchecked(out_pos);
                        str = buff_span_const{ 
							str_pos, 
							static_cast<std::size_t>(str_end - str_pos) };
                        return;
                    }
                }
                *out_pos++ = *str_pos++;
            }
            if (str_pos == str_end) {
                this->set_output_ptr_unchecked(out_pos);
                str = buff_span_const{ str_end, 0 };
            }
            else {
                this->set_error(__FILE__, __LINE__, error_code::no_space_in_buffer);
            }
        }

        void process_message() noexcept {
			buff_span_const msg_template{ msg_template_buffer() };
			bool has_repl_field = true;
			bool has_input = this->seek_field(logfield::Args);

            do {
                auto form = this->get_default_format();
                write_literal_escaped(msg_template);

                if (!msg_template.empty()) {
                    auto replacement_field = get_replacement_field(msg_template);
                    auto form_spec = format_spec(replacement_field);
                    form = this->get_format(form_spec);
                }
                else {
                    has_repl_field = false;
                    if (has_input && this->output_has_space()) {
                        *this->output_ptr() = ' ';
                        this->advance_output_unchecked(1);
                    }
                }
                
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
        }

		buff_span_const log_format_str() noexcept {
			return buff_span_const{ 
				formatting_buffer_.data(), log_format_size_ };
		}

		buff_span msg_template_buffer() noexcept {
			return buff_span{
				formatting_buffer_.data() + msg_template_start_, 
				msg_template_size_ };
		}

		inline std::string_view get_repl_field_name(logfield field_id) noexcept {
			switch (field_id) {
				case logfield::Severity: return "severity";
				case logfield::Policy: return "policy";
				case logfield::Channel:	return "channel";
				case logfield::Timestamp: return "timestamp";
				case logfield::Thread: return "thread";
				case logfield::Logger: return "logger";
				case logfield::File: return "file";
				case logfield::Line: return "line";
				case logfield::Function: return "function";
				case logfield::Message:	return "message";
				default: return "invalid";
			}
		}

		alignas(constants::cache_ls_nosharing) std::array<unsigned char, 512> formatting_buffer_{ 0 };
		std::uint32_t log_format_size_{ static_cast<std::uint32_t>(formatting_buffer_.size()) };
		std::uint32_t msg_template_start_{ static_cast<std::uint32_t>(formatting_buffer_.size()) };
		std::uint32_t msg_template_size_{ 0 };
		std::uint32_t msg_template_capacity_{ 0 };
    };
}
