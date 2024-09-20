//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).

#include <iostream>
#include <thread>

#include <fstlog/core.hpp>
#include <fstlog/logger/logger.hpp>
#include <fstlog/logger/logger_mt.hpp>
#include <fstlog/logger/logger_st.hpp>
#if defined(__cpp_nontype_template_args) && __cpp_nontype_template_args >= 201911L
#include <fstlog/logger/logger_st_fix.hpp>
#endif
#include <fstlog/logger/log_macro.hpp>
#include <fstlog/sink/sink_sort.hpp>
#include <fstlog/formatter/formatter_txt.hpp>
#include <fstlog/output/output_console.hpp>

template<class logger_type>
void logger_thread(logger_type& logger) {
	for (int i = 0; i < 4; i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds{20});
		LOG_INFO(logger, "Test log, (num: {}).", i);
	}
}

int main()
{
	try {
		// create core
		fstlog::core my_core("my_core");
		std::cout << "fstlog version: " << my_core.version() << "\n\n";
		// create sink
		fstlog::sink my_sink = fstlog::sink_sort(
			fstlog::formatter_txt("{timestamp:.2%M:%S} [{logger:10}] [thr:{thread:>10}] {message}"),
			fstlog::output_console());
		// assign sink to core
		my_core.add_sink(my_sink);

		{
			// fstlog::logger (thread safe to log, uses thread_local, has thread safe and non thread safe methods!)
			fstlog::logger logger_1(my_core, "logger_1");
			LOG_INFO(logger_1, "Starting test fstlog::logger.");
			// We can use the same logger for all threads.
			auto thread_1 = std::thread(logger_thread<fstlog::logger>, std::ref(logger_1));
			auto thread_2 = std::thread(logger_thread<fstlog::logger>, std::ref(logger_1));
			auto thread_3 = std::thread(logger_thread<fstlog::logger>, std::ref(logger_1));
			auto thread_4 = std::thread(logger_thread<fstlog::logger>, std::ref(logger_1));
			thread_1.join();
			thread_2.join();
			thread_3.join();
			thread_4.join();
		}

		{
			// fstlog::logger_mt (thread safe, uses std::mutex)
			fstlog::logger_mt logger_2(my_core, "logger_2");
			LOG_INFO(logger_2, "Starting test fstlog::logger_mt.");
			// We can use the same logger for all threads.
			auto thread_1 = std::thread(logger_thread<fstlog::logger_mt>, std::ref(logger_2));
			auto thread_2 = std::thread(logger_thread<fstlog::logger_mt>, std::ref(logger_2));
			auto thread_3 = std::thread(logger_thread<fstlog::logger_mt>, std::ref(logger_2));
			auto thread_4 = std::thread(logger_thread<fstlog::logger_mt>, std::ref(logger_2));
			thread_1.join();
			thread_2.join();
			thread_3.join();
			thread_4.join();
		}

		{
			// fstlog::logger_st (non thread safe);
			fstlog::logger_st logger_3_1(my_core, "logger_3_1");
			logger_3_1.set_thread("thread_1");
			fstlog::logger_st logger_3_2(my_core, "logger_3_2");
			logger_3_2.set_thread("thread_2");
			fstlog::logger_st logger_3_3(my_core, "logger_3_3");
			logger_3_3.set_thread("thread_3");
			fstlog::logger_st logger_3_4(my_core, "logger_3_4");
			logger_3_4.set_thread("thread_4");

			LOG_INFO(logger_3_1, "Starting test fstlog::logger_st.");
			// We have to use separate loggers for the threads.
			auto thread_1 = std::thread(logger_thread<fstlog::logger_st>, std::ref(logger_3_1));
			auto thread_2 = std::thread(logger_thread<fstlog::logger_st>, std::ref(logger_3_2));
			auto thread_3 = std::thread(logger_thread<fstlog::logger_st>, std::ref(logger_3_3));
			auto thread_4 = std::thread(logger_thread<fstlog::logger_st>, std::ref(logger_3_4));
			thread_1.join();
			thread_2.join();
			thread_3.join();
			thread_4.join();
		}
		
#if defined(__cpp_nontype_template_args) && __cpp_nontype_template_args >= 201911L
		{
			// fstlog::logger_st_fix<> (non thread safe);
			fstlog::logger_st_fix <
				fstlog::small_string<16>{"logger_4_1"},
				fstlog::level::Info,
				1,
				fstlog::small_string<16>{"thread_1"} >
				logger_4_1(my_core);
			fstlog::logger_st_fix <
				fstlog::small_string<16>{"logger_4_2"},
				fstlog::level::Info,
				1,
				fstlog::small_string<16>{"thread_2"} >
				logger_4_2(my_core);
			fstlog::logger_st_fix <
				fstlog::small_string<16>{"logger_4_3"},
				fstlog::level::Info,
				1,
				fstlog::small_string<16>{"thread_3"} >
				logger_4_3(my_core);
			fstlog::logger_st_fix <
				fstlog::small_string<16>{"logger_4_4"},
				fstlog::level::Info,
				1,
				fstlog::small_string<16>{"thread_4"} >
				logger_4_4(my_core);

			LOG_INFO(logger_4_1, "Starting test fstlog::logger_st_fix<>.");
			// We have to use separate loggers for the threads.
			auto thread_1 = std::thread(logger_thread<decltype(logger_4_1)>, std::ref(logger_4_1));
			auto thread_2 = std::thread(logger_thread<decltype(logger_4_2)>, std::ref(logger_4_2));
			auto thread_3 = std::thread(logger_thread<decltype(logger_4_3)>, std::ref(logger_4_3));
			auto thread_4 = std::thread(logger_thread<decltype(logger_4_4)>, std::ref(logger_4_4));
			thread_1.join();
			thread_2.join();
			thread_3.join();
			thread_4.join();
		}
#endif
	}
	catch (const std::exception& ex) {
		std::cout << ex.what() << '\n';
	}
}
