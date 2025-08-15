//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <cstddef> // __cpp_lib_format is defined here in windows
#if not defined(__cpp_lib_format) && (defined(__cplusplus) && __cplusplus >= 202000L)
#include <format> // __cpp_lib_format is defined here in gcc if __cplusplus >= 202000L
#endif
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
            "{timestamp} {level:} {message:.<10.5}",    
            // timestamp seconds in 2 decimal precision    
            "{timestamp:.2} {level} {message}", 
            // timestamp in UTC (precision 2)        
            "{timestamp:.2U} {level} {message}",
            // timestamp in UTC with strftime formatting (precision 2)
            // first U/L determines UTC/Local zone and is not printed
            "{timestamp:.2UUTC:%H:%M:%S +0000} {level} {message}", 
            // seconds in 0 decimal precision
            "{timestamp:.0} {level} {message}",
            // fill align with timestamp (precision 3 custom strftime format)
            "{timestamp:*^20.3%H:%M:%S} {level} {message}",
            // all fields
            "{timestamp} {level} {policy} {channel} {logger} {thread} {file}:{line} {function} {message}",
            // using '{' '}' chars in format string (usage inside replacement field is invalid)
            "{{{timestamp}}} {{}}{level} {{message:}}{message:}"
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
// if std::format is available 
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
    // create sink with the supplied formatter
    fstlog::sink my_sink = fstlog::sink_sort(
        formatter,
        fstlog::output_console());
    // retrieve the core that the logger is linked to
    auto core{ logger.get_core() };
    // assign sink to the core
    core.add_sink(my_sink);

    LOG_INFO(logger, "Hello {}!", "World");
    
    // flush sink
    core.flush();
    // remove sink from core
    core.release_sink(my_sink);
}
