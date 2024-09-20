//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/formatter/formatter_stdformat.hpp>

#include <cstddef>

#include <config_formatter_txt.hpp>
#include <detail/byte_span.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <fstlog/detail/noexceptions.hpp>

#if defined(__cpp_lib_format) && !defined(FSTLOG_NOEXCEPTIONS)
#include <detail/make_allocated.hpp>
#include <detail/mixin/allocator_mixin.hpp>
#include <detail/mixin/error_state_mixin.hpp>
#include <detail/mixin/exclusive_use_mixin.hpp>
#include <detail/mixin/reference_counter_mixin.hpp>
#include <formatter/formatter_interface_mixin.hpp>
#include <formatter/impl/arg_parser_mixin.hpp>
#include <formatter/impl/detail/decoder_internal_mixin.hpp>
#include <formatter/impl/detail/encoder_aggregate_separator_txt_mixin.hpp>
#include <formatter/impl/detail/encoder_stdformat_mixin.hpp>
#include <formatter/impl/detail/hash_converter_null_mixin.hpp>
#include <formatter/impl/formatter_txt_mixin.hpp>
#include <formatter/impl/header_internal_mixin.hpp>
#include <formatter/impl/input_span_mixin.hpp>
#include <formatter/impl/logfield_formspec_fmt_mixin.hpp>
#include <formatter/impl/logfield_pos_mixin.hpp>
#include <formatter/impl/output_span_mixin.hpp>
#endif

namespace fstlog {
#if defined(__cpp_lib_format) && !defined(FSTLOG_NOEXCEPTIONS)
	using formatter_stdformat_type =
        formatter_interface_mixin<
        formatter_txt_mixin<
        arg_parser_mixin<
        encoder_stdformat_mixin<
        encoder_aggregate_separator_txt_mixin<
        hash_converter_null_mixin<
        output_span_mixin<
        logfield_pos_mixin<
        decoder_internal_mixin<
        header_internal_mixin<
        input_span_mixin<
        logfield_formspec_fmt_mixin<
        error_state_mixin<
        exclusive_use_mixin<
		reference_counter_mixin<
        allocator_mixin>>>>>>>>>>>>>>>;

	static const char* formatter_stdformat(
		formatter& out,
		buff_span_const format_string,
		fstlog_allocator const& allocator) noexcept 
	{
		out = make_allocated<formatter_stdformat_type>(allocator);
		if (out.pimpl() == nullptr) return "Allocation failed (formatter obj.)!";
		auto error = static_cast<formatter_stdformat_type*>(out.pimpl())->
			formatter_init(format_string);
		if (error != nullptr) out = formatter{};
		return error;
	}
#else
	static const char* formatter_stdformat(
		[[maybe_unused]] formatter& out,
		[[maybe_unused]] buff_span_const format_string,
		[[maybe_unused]] fstlog_allocator const& allocator) noexcept
	{
#if defined(__cpp_lib_format)
		return "formatter_stdformat not available (exceptions disabled)!";
#else
		return "formatter_stdformat not available (std::format not available)!";
#endif
	}
#endif

	const char* formatter_stdformat(
		formatter& out, 
		std::string_view format_string,
        fstlog_allocator const& allocator) noexcept 
	{
		return formatter_stdformat(
			out,
			buff_span_const{
				safe_reinterpret_cast<const unsigned char*>(format_string.data()),
				format_string.size() },
			allocator);
    }

	const char* formatter_stdformat(
		formatter& out,
		fstlog_allocator const& allocator) noexcept
	{
        return formatter_stdformat(
			out, 
			config::default_format_string,
			allocator);
    }
}
