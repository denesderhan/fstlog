//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).

// the fstlog::filter is used for filtering log messages
// all sink use a filter
// Filtering is done by the log level and channel.
// All loggers have a channel setting, the channel is a number between 0-255
// the filter can filter multiple channels.
#include <iostream>
#include <sstream>

#include <fstlog/core.hpp>
#include <fstlog/logger/logger.hpp>
#include <fstlog/logger/log_macro.hpp>
#include <fstlog/sink/sink_sort.hpp>
#include <fstlog/formatter/formatter_txt.hpp>
#include <fstlog/output/output_stream.hpp>
#include <fstlog/filter/filter.hpp>

int main()
{
	try {
		// create core
		fstlog::core my_core("my_core");
		std::cout << "fstlog version: " << my_core.version() << "\n\n";

		// create filter that passes messages
		// with level::Info and above (Info, Warn, Error, Fatal)
		// and channel 1
		fstlog::filter filter_1(fstlog::level::Info, 1);
		
		// create filter that passes messages
		// with all levels
		// and all channels except 0, (channel 0 is used by fstlog for self logging)
		fstlog::filter filter_2(fstlog::level::All, 0, 255);
		
		// filter with arbitrary level and channel settings
		fstlog::filter filter_3;
		// adding individual log level
		filter_3.add_level(fstlog::level::Debug);
		// adding range of log levels
		filter_3.add_level(fstlog::level::Warn, fstlog::level::Fatal);
		// adding individual channel
		filter_3.add_channel(1);
		// adding range of channels
		filter_3.add_channel(10, 20);

		// create output streams
		auto stream_1 = std::make_shared<std::stringstream>();
		fstlog::output out_1 = fstlog::output_stream(stream_1);
		auto stream_2 = std::make_shared<std::stringstream>();
		fstlog::output out_2 = fstlog::output_stream(stream_2);
		auto stream_3 = std::make_shared<std::stringstream>();
		fstlog::output out_3 = fstlog::output_stream(stream_3);
		

		//create sinks 
		fstlog::sink sink_1 = fstlog::sink_sort(
			fstlog::formatter_txt("from sink_1 {time:.2%H:%M:%S} [{level:5}] [channel: {channel:3}] [{logger}] {message}"),
			out_1,
			filter_1);
		fstlog::sink sink_2 = fstlog::sink_sort(
			fstlog::formatter_txt("from sink_2 {time:.2%H:%M:%S} [{level:5}] [channel: {channel:3}] [{logger}] {message}"),
			out_2,
			filter_2);
		fstlog::sink sink_3 = fstlog::sink_sort(
			fstlog::formatter_txt("from sink_3 {time:.2%H:%M:%S} [{level:5}] [channel: {channel:3}] [{logger}] {message}"),
			out_3,
			filter_3);

		//add sinks to core
		my_core.add_sink(sink_1);
		my_core.add_sink(sink_2);
		my_core.add_sink(sink_3);

		//create loggers, set channels
		fstlog::logger logger_1(my_core, "logger_1");
		logger_1.set_channel(1);
		fstlog::logger logger_2(my_core, "logger_2");
		logger_2.set_channel(2);
		fstlog::logger logger_3(my_core, "logger_3");
		logger_3.set_channel(15);

		//log with loggers
		LOG_TRACE(logger_1, "Test.");
		LOG_TRACE(logger_2, "Test.");
		LOG_TRACE(logger_3, "Test.");
		LOG_DEBUG(logger_1, "Test.");
		LOG_DEBUG(logger_2, "Test.");
		LOG_DEBUG(logger_3, "Test.");
		LOG_INFO(logger_1, "Test.");
		LOG_INFO(logger_2, "Test.");
		LOG_INFO(logger_3, "Test.");
		LOG_WARN(logger_1, "Test.");
		LOG_WARN(logger_2, "Test.");
		LOG_WARN(logger_3, "Test.");
		LOG_ERROR(logger_1, "Test.");
		LOG_ERROR(logger_2, "Test.");
		LOG_ERROR(logger_3, "Test.");
		LOG_FATAL(logger_1, "Test.");
		LOG_FATAL(logger_2, "Test.");
		LOG_FATAL(logger_3, "Test.");

		//stopping core to flush logs to streams ( or call my_core.flush() )
		my_core.stop();

		std::cout << "Filtered by filter_1 (level INFO - FATAL, channel: 1):\n" << stream_1->str() << "\n\n";
		std::cout << "Filtered by filter_2 (level TRACE - FATAL, channel: 0 - 255):\n" << stream_2->str() << "\n\n";
		std::cout << "Filtered by filter_3 (level DEBUG, WARN - FATAL, channel: 1, 10 - 20):\n" << stream_3->str() << "\n\n";
	}
	catch (const std::exception& ex) {
		std::cout << ex.what() << '\n';
	}
}
