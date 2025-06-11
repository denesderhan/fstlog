//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <array>
#include <cstdint>
#include <regex>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include <fstlog/core.hpp>
#include <fstlog/logger/logger.hpp>
#include <fstlog/logger/log_macro.hpp>
#include <fstlog/sink/sink_unsort.hpp>
#include <fstlog/formatter/formatter_txt.hpp>
#include <fstlog/output/output_stream.hpp>

#include <config_sink.hpp>
#include <detail/make_allocated.hpp>
#include <detail/mixin/allocator_mixin.hpp>
#include <detail/mixin/exclusive_use_mixin.hpp>
#include <detail/mixin/reference_counter_mixin.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/filter/filter.hpp>
#include <filter/filter_impl.hpp>
#include <filter/filter_internal.hpp>
#include <sink/impl/sink_unsort_mixin.hpp>
#include <filter/filter_mixin.hpp>
#include <sink/sink_flush_time_mixin.hpp>
#include <sink/sink_formatter_mixin.hpp>
#include <sink/sink_interface_mixin.hpp>
#include <sink/sink_msgblock_mixin.hpp>
#include <sink/sink_output_mixin.hpp>

namespace fstlog {
	using sink_small_impl_type =
		sink_interface_mixin<
		sink_msgblock_mixin<
		sink_unsort_mixin<
		sink_formatter_mixin<128,
		sink_output_mixin<
		filter_mixin<
		sink_flush_time_mixin<
		reference_counter_mixin<
		exclusive_use_mixin<
		allocator_mixin>>>>>>>>>;

	static error_code sink_small(
		sink& out,
		formatter formatter,
		output output,
		filter_internal const& filter,
		std::chrono::milliseconds flush_interval,
		fstlog_allocator const& allocator) noexcept
	{
		out = make_allocated<sink_small_impl_type>(allocator);
		sink_small_impl_type* const pimpl =
			static_cast<sink_small_impl_type*>(out.pimpl());
		if (pimpl == nullptr) return error_code::alloc_fail;
		auto error = pimpl->set_formatter(std::move(formatter));
		if (error == error_code::none) error = pimpl->set_output(std::move(output));
		if (error != error_code::none) {
			out = sink{};
			return error;
		}
		pimpl->set_filter(filter);
		pimpl->set_flush_interval(flush_interval);
		return error_code::none;
	}
}

TEST_CASE("formatter_txt_mixin") {
	
	SECTION("init") {
		auto test_dat = GENERATE(
			std::make_tuple(std::string_view{ "" }, fstlog::error_code::none),
			std::make_tuple(std::string_view{ " " }, fstlog::error_code::fmt_bad),
			std::make_tuple(std::string_view{ "{message}{" }, fstlog::error_code::fmt_bad),
			std::make_tuple(std::string_view{ "{bad} {message}" }, fstlog::error_code::fmt_bad),
			std::make_tuple(std::string_view{ "{time} {message}" }, fstlog::error_code::none),
			std::make_tuple(std::string_view{ "{time} {time} {message}" }, fstlog::error_code::double_init),
			std::make_tuple(std::string_view{ "{message} {message}" }, fstlog::error_code::none),
			std::make_tuple(std::string_view{ "{ {message}" }, fstlog::error_code::fmt_bad),
			std::make_tuple(std::string_view{ "{{message}" }, fstlog::error_code::fmt_bad),
			std::make_tuple(std::string_view{ "}{message}" }, fstlog::error_code::fmt_bad),
			std::make_tuple(std::string_view{ "{message}} " }, fstlog::error_code::fmt_bad),
			std::make_tuple(std::string_view{ "{message}{ " }, fstlog::error_code::fmt_bad),
			std::make_tuple(std::string_view{ "{message:\xf8}" }, fstlog::error_code::fmt_bad),
			std::make_tuple(std::string_view{ "{message:\xc2\xa9^ 9999.9999A}" }, fstlog::error_code::none),
			std::make_tuple(std::string_view{ "{message:\xc2\xa9^*9999.9999A}" }, fstlog::error_code::fmt_bad),
			std::make_tuple(std::string_view{ "{message:^^9999.9999s}" }, fstlog::error_code::none),
			std::make_tuple(std::string_view{ "{message:+.999}" }, fstlog::error_code::none),
			std::make_tuple(std::string_view{ "{timestamp:*<30.2} {message}" }, fstlog::error_code::none),
			std::make_tuple(std::string_view{ "{timestamp:*<-30.2} {message}" }, fstlog::error_code::fmt_bad),
			std::make_tuple(std::string_view{ "{timestamp:*< 30.2} {message}" }, fstlog::error_code::fmt_bad),
			std::make_tuple(std::string_view{ "{timestamp:*<#30.2} {message}" }, fstlog::error_code::fmt_bad),
			std::make_tuple(std::string_view{ "{timestamp:*<030.2} {message}" }, fstlog::error_code::fmt_bad),
			std::make_tuple(std::string_view{ "{timestamp:*<30.2%T} {message}" }, fstlog::error_code::fmt_bad),
			std::make_tuple(std::string_view{ "{timestamp:*<30.2%Y-%m-%d %H:%M:%S} {message}" }, fstlog::error_code::none),
			std::make_tuple(std::string_view{ "{timestamp:*<30.2L%Y-%m-%d %H:%M:%S%%} {message}" }, fstlog::error_code::none),
			std::make_tuple(std::string_view{ "{timestamp:*<30.2%Y-%m-%d %H:%M:%S%:} {message}" }, fstlog::error_code::fmt_bad)
		);
		
		fstlog::formatter f;
		auto error = fstlog::formatter_txt(f, std::get<0>(test_dat));
		std::string_view str = std::get<0>(test_dat);
		CAPTURE(str);
		CHECK(error == std::get<1>(test_dat));
	}

	SECTION("non_matching_repl_fields_data") {
		fstlog::core core;
		fstlog::logger logger(core);
		auto out_str = std::make_shared<std::ostringstream>(std::stringstream::binary);
		
		auto test_dat = GENERATE(
			std::make_tuple(std::string_view{ "{}{}" }, std::string_view{ "HelloWorld\n" }),
			std::make_tuple(std::string_view{ "{} {}!" }, std::string_view{ "Hello World!\n" }),
			std::make_tuple(std::string_view{ "{}" }, std::string_view{ "Hello... Message truncated, fstlog error: Invalid fmt format string!" }),
			std::make_tuple(std::string_view{ "{} {} {}" }, std::string_view{ "Hello World ... Message truncated, fstlog error: Input data was malformed or corrupted!" })
		);
		
		auto sink = fstlog::sink_unsort(
			fstlog::formatter_txt("{message}"),
			fstlog::output_stream(out_str));
		CHECK(sink.good());
		core.add_sink(sink);
		LOG_INFO(logger, std::get<0>(test_dat), "Hello", "World");
		core.flush();
		core.release_sink(sink);
		CHECK(out_str->str().length() >= std::get<1>(test_dat).length());
		std::string trunc_str = out_str->str();
		trunc_str.resize(std::get<1>(test_dat).length());
		CAPTURE(trunc_str);
		CHECK(trunc_str == std::get<1>(test_dat));
		out_str->str("");
		
	}

	SECTION("fields") {
		fstlog::core core;
		fstlog::logger logger(core, "test_logger");
		auto out_str = std::make_shared<std::ostringstream>(std::stringstream::binary);
		auto sink = fstlog::sink_unsort(
			fstlog::formatter_txt(
				"{timestamp:.3%Y-%m-%d %H:%M:%S}|{severity}|{policy}|{channel}|{thread}|{logger}|{file}|{line}|{function}|{message}"),
			fstlog::output_stream(out_str));
		CHECK(sink.good());
		core.add_sink(sink);
		LOG_INFO(logger, "Test message.");
		core.flush();
		core.release_sink(sink);
		//std::string log_str = out_str->str();
		//CAPTURE(log_str);
		
		std::vector<std::string> fields;
		std::string token;
		std::stringstream temp(out_str->str());
		while (std::getline(temp, token, '|')) {
			fields.push_back(token);
		}
		REQUIRE(fields.size() == 10);
		
		CHECK(std::regex_match(fields[0], std::regex(R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}$)")));
		CHECK(fields[1] == "INFO");
		CHECK(fields[2] == "Guaranteed");
		CHECK(fields[3] == "1");
		auto thread_str = fstlog::this_thread::get_str();
		CHECK(fields[4] == std::string_view(thread_str));
		CHECK(fields[5] == "test_logger");
		CHECK(std::regex_match(fields[6], std::regex(R"(.*formatter_txt_mixin.cpp)")));
		CHECK(std::regex_match(fields[7], std::regex(R"(\d+)")));
		CHECK(!fields[8].empty());
		CHECK(fields[9] == "Test message.\n");
	}

	SECTION("no_space_in_buffer") {
		fstlog::core core;
		fstlog::sink out_sink;
		auto out_str = std::make_shared<std::ostringstream>(std::stringstream::binary);
		auto output = fstlog::output_stream(out_str);
		auto sink_error = fstlog::sink_small(
			out_sink,
			fstlog::formatter_txt("{message}"),
			output,
			fstlog::filter_internal{ fstlog::level::All, 1, 255 },
			fstlog::config::default_sink_flush_interval,
			fstlog::fstlog_allocator{});
		CHECK(sink_error == fstlog::error_code::none);
		core.add_sink(out_sink);
		fstlog::logger logger(core);

		LOG_INFO(logger, "This will not fit in the sinks formatting buffer of size 128 bytes! This will not fit in the sinks formatting buffer of size 128 bytes!");
		core.flush();
		CHECK(out_str->str() == "This will not fit in the sinks formatting buffer of size 128 bytes! This will not fit in the sinks formatting buffer of size 12\n");
		out_str->str("");
		LOG_INFO(logger, "This will not fit in the sinks formatting buffer of size 128 bytes! This will not fit in the sinks formatting buffer {}", 1111111111111LL );
		core.flush();
		CHECK(out_str->str() == "This will not fit in the sinks formatting buffer of size 128 bytes! This will not fit in the sinks formatting buffer Err:002\n");
		out_str->str("");
		LOG_INFO(logger, "This will not fit in the sinks formatting buffer of size 128 bytes! {}", "This will not fit in the sinks formatting buffer of size 128 bytes!");
		core.flush();
		CHECK(out_str->str() == "This will not fit in the sinks formatting buffer of size 128 bytes! ... fstlog error:002\n");
		out_str->str("");
		LOG_INFO(logger, "String: {}", "This will not fit in the sinks formatting buffer of size 128 bytes! This will not fit in the sinks formatting buffer of size 128 bytes!");
		core.flush();
		CHECK(out_str->str() == "String: ... Message truncated, fstlog error: Not enough space in output buffer!\n");

	};
	
}
