//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).

#include <iostream>
#include <string_view>

#include <fstlog/core.hpp>
#include <fstlog/sink/sink_sort.hpp>
#include <fstlog/formatter/formatter_txt.hpp>
#include <fstlog/formatter/formatter_txt_fast.hpp>
#include <fstlog/formatter/formatter_stdformat.hpp>
#include <fstlog/output/output_console.hpp>
#include <fstlog/logger/logger.hpp>
#include <fstlog/logger/log_macro.hpp>

void log_with_formatter(fstlog::logger& logger, fstlog::formatter formatter);

int main()
{
	try {
		//Format patterns
		std::vector<std::string_view> format_patterns{
			// default pattern
			"",
			// fill align
			"{timestamp} {level:*<10} {message:*>20}", 
			// truncation	
			"{message:.5}",	
			// timestamp in UTC			
			"{timestamp:U}",
			// timestamp in UTC (first U/L determines UTC/local zone and is consumed), custom timestamp (strftime)
			"{timestamp:UUTC:%H:%M:%S +0000}", 
			// seconds in 2 decimal precision	
			"{timestamp:.2%H:%M:%S %z}", 
			// seconds in 0 decimal precision
			"{timestamp:.0%H:%M:%S %z}",
			// fill align with timestamp
			"{timestamp:*^20.3%H:%M:%S}",
			// all fields
			"{timestamp} {level} {policy} {channel} {logger} {thread} {file}:{line} {function} {message}"
		};

		// create core
		fstlog::core my_core{ "my_core" };
		std::cout << "fstlog version: " << my_core.version() << "\n\n";
		// create logger
		fstlog::logger my_logger{ my_core, "my_logger" };
		
		for (auto pattern : format_patterns) {
			std::cout << "------------------------------------------------------------\n";
			std::cout << "Using format pattern: [" << pattern << "]\n";
			std::cout << "------------------------------------------------------------\n";
			// formatter_txt
			std::cout << "[formatter_txt]\n";
			log_with_formatter(my_logger, fstlog::formatter_txt(pattern));
			std::cout << '\n';
// if std::format available 
#if defined(__cpp_lib_format)
			// formatter_txt_stdformat
			std::cout << "[formatter_stdformat]\n";
			log_with_formatter(my_logger, fstlog::formatter_stdformat(pattern));
			std::cout << '\n';
#endif
			// formatter_txt_fast
			std::cout << "[formatter_txt_fast]\n";
			log_with_formatter(my_logger, fstlog::formatter_txt_fast(pattern));
			std::cout << '\n';
		}
	}
	catch (const std::exception &ex) {
		std::cout << "ERROR, exception:" << ex.what() << '\n';
		return 1;
	}
}

void log_with_formatter(fstlog::logger& logger, fstlog::formatter formatter) {
	//create sink
	fstlog::sink my_sink = fstlog::sink_sort(
		formatter,
		fstlog::output_console());
	auto core{ logger.get_core() };
	//assign sink to core
	core.add_sink(my_sink);
	LOG_INFO(logger, "Hello {}!", "World");
	//flush sink
	core.flush();
	//remove sink from core
	core.release_sink(my_sink);
}
