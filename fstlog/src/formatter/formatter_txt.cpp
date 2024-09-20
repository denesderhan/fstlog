//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/formatter/formatter_txt.hpp>

#include <cstddef>
#include <new>

#include <config_formatter_txt.hpp>
#include <detail/byte_span.hpp>
#include <detail/make_allocated.hpp>
#include <detail/mixin/allocator_mixin.hpp>
#include <detail/mixin/error_state_mixin.hpp>
#include <detail/mixin/exclusive_use_mixin.hpp>
#include <detail/mixin/reference_counter_mixin.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <formatter/formatter_interface_mixin.hpp>
#include <formatter/impl/arg_parser_mixin.hpp>
#include <formatter/impl/detail/decoder_internal_mixin.hpp>
#include <formatter/impl/detail/encoder_aggregate_separator_txt_mixin.hpp>
#include <formatter/impl/detail/encoder_charconv_mixin.hpp>
#include <formatter/impl/detail/hash_converter_null_mixin.hpp>
#include <formatter/impl/formatter_txt_mixin.hpp>
#include <formatter/impl/header_internal_mixin.hpp>
#include <formatter/impl/input_span_mixin.hpp>
#include <formatter/impl/logfield_formspec_txt_mixin.hpp>
#include <formatter/impl/logfield_pos_mixin.hpp>
#include <formatter/impl/output_span_mixin.hpp>

namespace fstlog {
	using formatter_txt_type = 
		formatter_interface_mixin<
		formatter_txt_mixin<
		arg_parser_mixin<
		encoder_charconv_mixin<
		encoder_aggregate_separator_txt_mixin<
		hash_converter_null_mixin<
		output_span_mixin<
		logfield_pos_mixin<
		decoder_internal_mixin<
		header_internal_mixin<
		input_span_mixin<
		logfield_formspec_txt_mixin<
		error_state_mixin<
		exclusive_use_mixin<
		reference_counter_mixin<
		allocator_mixin>>>>>>>>>>>>>>>;
	
	static const char* formatter_txt(
		formatter& out,
		buff_span_const format_string,
		fstlog_allocator const& allocator) noexcept
	{
		out = make_allocated<formatter_txt_type>(allocator);
		if (out.pimpl() == nullptr) return "Allocation failed (formatter obj.)!";
		auto error = static_cast<formatter_txt_type*>(out.pimpl())->
			formatter_init(format_string);
		if (error != nullptr) out = formatter{};
		return error;
	}

	const char* formatter_txt(
		formatter& out,
		std::string_view format_string,
		fstlog_allocator const& allocator) noexcept
	{
		return formatter_txt(
			out,
			buff_span_const{ 
				safe_reinterpret_cast<const unsigned char*>(format_string.data()),
				format_string.size() },
			allocator);
	}

	const char* formatter_txt(
		formatter& out,
		fstlog_allocator const& allocator) noexcept
	{
		return formatter_txt(
			out,
			config::default_format_string,
			allocator);
	}
}
