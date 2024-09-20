//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).

#include <iostream>
#ifdef _WIN32
#include <windows.h>
#pragma execution_character_set( "utf-8" )
#endif

#include <vector>
#include <utility>
#include <tuple>

#include <fstlog/core.hpp>
#include <fstlog/logger/logger.hpp>
#include <fstlog/logger/log_macro.hpp>
#include <fstlog/sink/sink_sort.hpp>
#include <fstlog/formatter/formatter_txt.hpp>
#include <fstlog/output/output_console.hpp>

template<class C>
class useless_container {
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
	try {
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
		void* ptr{ nullptr };
		const unsigned long long num{ 1234 };
		LOG_INFO(my_logger, "Logging fundamental types, int: {}, unsigned long long: {}, double: {}.", -1, num, 0.1);
		LOG_INFO(my_logger, "Logging fundamental types, pointer: {}, bool: {}.", ptr, true);
		LOG_INFO(my_logger, "Logging fundamental types, signed char: {}, unsigned char: {}, char: {}.",
			(signed char)-128, (unsigned char)255, 'A');
#ifdef __cpp_char8_t
		LOG_INFO(my_logger, "Logging fundamental types, char8_t: {}, char8 string: {}.", char8_t(0x40), u8"¡¢£¤¥¦§");
#endif
		LOG_INFO(my_logger, "Logging fundamental types, char16_t: {}, char32_t: {}.", char16_t('A'), char32_t('B'));
		LOG_INFO(my_logger, "Logging strings: {} {}, {}.", "Hello", u"World§", U"Hello§");

		using namespace std::string_literals;
		auto s1{ U"Logging strings: {} {}."s };
		auto s2{ u"Hello§"s };
		auto s3{ std::basic_string_view{U"World§"} };
		LOG_INFO(my_logger, s1, s2, s3);

		// logging works with arbitrary message types
		LOG_INFO(my_logger, 42);
		LOG_INFO(my_logger, (float)0.123);

		// Tuples are loggable
		LOG_INFO(my_logger, "Logging tuple: {}.", std::tuple<bool, int>{true, 1});

		// Logging containers
		std::vector<void*> var1{ &s1, &s2 };
		std::vector<std::pair<int, bool>> var2{ {0, false}, {1, true} };
		LOG_INFO(my_logger, "Logging containers: vector: {}, vector of pairs: {}", var1, var2);
		std::tuple<std::vector<int>, std::pair<bool, float>, int> var3{ {1, 2}, {true, 1.5f}, 2 };
		LOG_INFO(my_logger, "Logging containers: std::tuple<std::vector<int>, std::pair<bool, float>: {}", var3);

		// All containers are loggable that have a value_type a size() method
		// an iterator and contain loggable types
		useless_container<std::vector<int>> var4{ {{1, 2}, {3, 4}} };
		LOG_INFO(my_logger, "Logging custom containers: {}", var4);
		useless_container<useless_container<std::vector<int>>> var5{ {var4, var4} };
		LOG_INFO(my_logger, "Logging custom containers: {}", var5);

		// Formatting messages
		// the syntax of std::format is used, (available formatting options are dependent on formatter type)
		// align message parameters with filler chars
		LOG_INFO(my_logger, "Logging aligned: {:.>10}, {:¤^10}", 3, "TEXT");
		// number formatting
		LOG_INFO(my_logger, "Number formatting: hex: {:#X}, binary: {:+#b}, precision: {:.2}, scientific: {:.3e}", -10, (signed char)10, 1.23456f, 100000.5f);
	
		// control characters are replaced with '_' in the formatted log message
		LOG_INFO(my_logger, "\tLog\a\b\r injection attack!: {} ", "\nThis should be in the same line!");
	}
	catch (const std::exception& ex) {
		std::cout << ex.what() << '\n';
	}
}
