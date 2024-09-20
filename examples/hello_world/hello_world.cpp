//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).

#include <iostream>

#include <fstlog/core.hpp>
#include <fstlog/logger/logger.hpp>
#include <fstlog/logger/log_macro.hpp>
#include <fstlog/sink/sink_sort.hpp>
#include <fstlog/formatter/formatter_txt.hpp>
#include <fstlog/output/output_console.hpp>

int main()
{
	try {
		//create core
		fstlog::core my_core("my_core");
		std::cout << "fstlog version: " << my_core.version() << "\n\n";
		//create sink
		fstlog::sink my_sink = fstlog::sink_sort(
			fstlog::formatter_txt(),
			fstlog::output_console());
		//assign sink to core
		my_core.add_sink(my_sink);

		//create logger
		fstlog::logger my_logger(my_core);
		
		//log with logger
		LOG_INFO(my_logger, "Hello {}!", "World");
	}
	catch (const std::exception& ex) {
		std::cout << ex.what() << '\n';
	}
}
