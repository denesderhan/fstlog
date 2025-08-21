//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).

#ifdef _WIN32
#include <windows.h>
#pragma execution_character_set( "utf-8" )
#endif

#include <iostream>
#include <limits>
#include <tuple>
#include <utility>
#include <vector>

#include <fstlog/core.hpp>
#include <fstlog/formatter/formatter_txt.hpp>
#include <fstlog/logger/log_macro.hpp>
#include <fstlog/logger/logger.hpp>
#include <fstlog/output/output_console.hpp>
#include <fstlog/sink/sink_sort.hpp>

template<class C>
class example_container {
public:
    using value_type = C;
    using container = std::vector<C>;
    using const_iterator = typename container::const_iterator;

    std::size_t size() const {
        return var_.size();
    }
    const_iterator begin() const { return var_.begin(); }
    const_iterator end() const { return var_.end(); }

    std::vector<C> var_;
};

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    //create core
    fstlog::core my_core("my_core");
    std::cout << "fstlog version: " << my_core.version() << "\n\n";

    //create sink
    fstlog::sink my_sink = fstlog::sink_sort(
        fstlog::formatter_txt("{time} {level} {message}"),
        fstlog::output_console());
    //assign sink to core
    my_core.add_sink(my_sink);

    //create logger
    fstlog::logger my_logger(my_core);

    //fundamental types
    LOG_INFO(my_logger, "Logging fundamental types.");
    LOG_INFO(my_logger, "unsigned/signed char: {}/{}, short: {}/{}, int: {}/{}",
        (std::numeric_limits<unsigned char>::max)(), (std::numeric_limits<signed char>::min)(), 
        (std::numeric_limits<unsigned short>::max)(), (std::numeric_limits<signed short>::min)(),
        (std::numeric_limits<unsigned int>::max)(), (std::numeric_limits<signed int>::min)());
    LOG_INFO(my_logger, "unsigned/signed long: {}/{}, long long: {}/{}",
        (std::numeric_limits<unsigned long>::max)(), (std::numeric_limits<signed long>::min)(),
        (std::numeric_limits<unsigned long long>::max)(), (std::numeric_limits<signed long long>::min)());
    LOG_INFO(my_logger, "float: {}, double: {}", 0.1f, 0.2);
        
    void* ptr{ nullptr };
    LOG_INFO(my_logger, "pointer: {}, bool: {}", ptr, true);
        
    LOG_INFO(my_logger, "Logging characters as text, char (ASCII): {}, char16_t (unicode < 65536): {}, char32_t (unicode): {}",
        char('A'), char16_t(u'¥'), char32_t(U'£'));
        
    LOG_INFO(my_logger, "Logging characters as bytes, char: {:#X}, char16_t: {:#X}, char32_t: {:#X}",
        char('A'), char16_t(u'¥'), char32_t(U'£'));
        
    LOG_INFO(my_logger, "Escaping invalid/unsafe characters: char: {}, char16_t: {}, char32_t: {}",
        char(0), char16_t(0xFFFD), char32_t(0x13000));
        
    LOG_INFO(my_logger, std::basic_string_view<char32_t>(U"Logging strings."));

    LOG_INFO(my_logger, "utf-8 string in char type: {}.", "¤$€£¥");
#ifdef __cpp_char8_t
    LOG_INFO(my_logger, "urf-8 string in char8_t type: {}.", u8"¤$€£¥");
#endif
    LOG_INFO(my_logger, "char16_t, char32_t strings: {} {}", u"Hello ʘ", U"World ʘ");

    // logging works with arbitrary message types
    LOG_INFO(my_logger, 42);
    LOG_INFO(my_logger, (float)0.123);

    // Tuples are loggable
    LOG_INFO(my_logger, "Logging tuple: {}.", std::tuple<bool, int>{true, 1});

    // Logging containers
    std::tuple<std::vector<int>, std::pair<bool, float>, int> var3{ {1, 2}, {true, 1.5f}, 2 };
    LOG_INFO(my_logger, "Logging containers: std::tuple<std::vector<int>, std::pair<bool, float>, int>: {}", var3);

    // All containers are loggable that have: a value_type, a size() method, an iterator
    // and contain loggable types
    example_container<std::vector<int>> var4{ {{1, 2}, {3, 4}} };
    LOG_INFO(my_logger, "Logging custom containers: {}", var4);
    example_container<example_container<std::vector<int>>> var5{ {var4, var4} };
    LOG_INFO(my_logger, "Logging custom containers: {}", var5);

    // Formatting messages
    // the syntax of std::format is used, (available formatting options are dependent on formatter type)
    // align message parameters with filler chars
    LOG_INFO(my_logger, "Logging aligned: {:.>10}, {:ʘ^10}", 3, "TEXT");
    // number formatting
    LOG_INFO(my_logger, "Number formatting: hex: {:#X}, binary: {:+#b}, precision: {:.2}, scientific: {:.3e}", -10, (signed char)10, 1.23456f, 100000.5f);
    
    // Unsafe unicode code points are escaped in the formatted log message.
    LOG_INFO(my_logger, "Escaping unsafe characters:\t\a {} ", "\b\r\nThis should be in the same line!");
}
