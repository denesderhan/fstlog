//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).

#include <iostream>

#include <fstlog/version.hpp>
#include <fstlog/core.hpp>
#include <fstlog/logger/logger.hpp>
#include <fstlog/logger/log_macro.hpp>
#ifdef __cpp_lib_source_location 
#include <fstlog/logger/log_sloc.hpp>
#endif
#include <fstlog/sink/sink_sort.hpp>
#include <fstlog/formatter/formatter_txt.hpp>
#include <fstlog/output/output_console.hpp>

int main()
{
    std::cout << "fstlog version: " << fstlog::version() << "\n\n";
    //create core
    fstlog::core my_core("my_core");
    //create sink
    fstlog::sink my_sink = fstlog::sink_sort(
        fstlog::formatter_txt("{time} {level} {policy} {file}:{line} {message}"),
        fstlog::output_console());
    //assign sink to core
    my_core.add_sink(my_sink);

    //create logger
    fstlog::logger my_logger(my_core);
        
    //log with logger

    LOG_INFO(my_logger, "Hello {}!", "Info");
    LOG_NG_ERROR(my_logger, "Hello {} {}!", "Non Guaranteed", "Error");
    LOG(my_logger, fstlog::level::Error, "Hello {} {}!", "Non fixed", "level");
    LOG_LL(my_logger, fstlog::level::Warn, "Hello {} {}!", "Low Latency", "Warn");
#ifdef __cpp_lib_source_location
    log_info(my_logger)("Hello {}!", "Info");
    log_error<fstlog::log_policy_nonguaranteed>(my_logger)("Hello {} {}!", "Non Guaranteed", "Error");
    log(my_logger, fstlog::level::Error)("Hello {} {}!", "Non fixed", "level");
    log<fstlog::log_policy_lowlatency>(my_logger, fstlog::level::Warn)("Hello {} {}!", "Low Latency", "Warn");
#endif
}
