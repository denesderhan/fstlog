//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <fstlog/core.hpp>
#include <fstlog/logger/log_macro.hpp>
#include <fstlog/logger/logger_st.hpp>

//for each SECTION the TEST_CASE is executed from the start! (preventing multiple core constructs)
inline fstlog::core core;

TEST_CASE("logger_st") {
	core.poll_interval(std::chrono::milliseconds{0});

	SECTION("construct") {
		fstlog::logger_st logger;
		CHECK(logger.get_core().pimpl() == nullptr);
		CHECK(logger.name() == fstlog::small_string<32>{"Unnamed"});
		CHECK(logger.channel() == 1);
		CHECK(logger.thread() == fstlog::small_string<32>{"Unnamed"});
		CHECK(logger.dropped() == 0);
		CHECK(logger.level() == fstlog::level::All);
		
		CHECK(logger.good() == false);
		logger.set_core(core);
		CHECK(logger.get_core().pimpl() == core.pimpl());
		CHECK(logger.good() == false);
		logger.new_buffer(1024);
		CHECK(logger.good() == true);

		logger.set_name("logger_1");
		logger.set_thread("thread_1");
		logger.set_level(fstlog::level::Debug);
		logger.set_channel(10);
		CHECK(logger.name() == fstlog::small_string<32>{"logger_1"});
		CHECK(logger.channel() == 10);
		CHECK(logger.thread() == fstlog::small_string<32>{"thread_1"});
		CHECK(logger.level() == fstlog::level::Debug);

		for (int i = 0; i < 10; i++) {
			LOG_LL_FATAL(logger, "Test text, test text, test text, test text, test text, test text, test text.");
		}
		CHECK(logger.dropped() > 0);

		const auto dropped_logs{ logger.dropped() };

		//copy construct/assign
		fstlog::logger_st logger_cpy{logger};
		fstlog::logger_st logger_cpy2;
		logger_cpy2 = logger;
		
		CHECK(logger.get_core().pimpl() == core.pimpl());
		CHECK(logger.name() == fstlog::small_string<32>{"logger_1"});
		CHECK(logger.channel() == 10);
		CHECK(logger.thread() == fstlog::small_string<32>{"thread_1"});
		CHECK(logger.level() == fstlog::level::Debug);
		CHECK(logger.dropped() == dropped_logs);

		CHECK(logger_cpy.get_core().pimpl() == core.pimpl());
		CHECK(logger_cpy.name() == fstlog::small_string<32>{"Unnamed"});
		CHECK(logger_cpy.channel() == 10);
		CHECK(logger_cpy.thread() == fstlog::small_string<32>{"Unnamed"});
		CHECK(logger_cpy.level() == fstlog::level::Debug);
		CHECK(logger_cpy.dropped() == 0);

		CHECK(logger_cpy2.get_core().pimpl() == core.pimpl());
		CHECK(logger_cpy2.name() == fstlog::small_string<32>{"Unnamed"});
		CHECK(logger_cpy2.channel() == 10);
		CHECK(logger_cpy2.thread() == fstlog::small_string<32>{"Unnamed"});
		CHECK(logger_cpy2.level() == fstlog::level::Debug);
		CHECK(logger_cpy2.dropped() == 0);

		logger_cpy.set_name("logger_2");
		CHECK(logger_cpy.name() == fstlog::small_string<32>{"logger_2"});
		logger_cpy.set_thread("thread_2");
		CHECK(logger_cpy.thread() == fstlog::small_string<32>{"thread_2"});
		for (int i = 0; i < 10; i++) {
			LOG_LL_FATAL(logger_cpy, "Test text, test text, test text, test text, test text, test text, test text.");
		}
		CHECK(logger_cpy.dropped() > 0);

		const auto dropped_logs_2{ logger_cpy.dropped() };

		//move construct/assign
		fstlog::logger_st logger_mov{ std::move(logger) };
		fstlog::logger_st logger_mov2;
		logger_mov2 = std::move(logger_cpy);

		CHECK(logger.get_core().pimpl() == nullptr);
		CHECK(logger.name() == fstlog::small_string<32>{"Unnamed"});
		CHECK(logger.channel() == 10);
		CHECK(logger.thread() == fstlog::small_string<32>{"Unnamed"});
		CHECK(logger.level() == fstlog::level::Debug);
		CHECK(logger.dropped() == 0);
		CHECK(logger_cpy.get_core().pimpl() == nullptr);
		CHECK(logger_cpy.name() == fstlog::small_string<32>{"Unnamed"});
		CHECK(logger_cpy.channel() == 10);
		CHECK(logger_cpy.thread() == fstlog::small_string<32>{"Unnamed"});
		CHECK(logger_cpy.level() == fstlog::level::Debug);
		CHECK(logger_cpy.dropped() == 0);

		CHECK(logger_mov.get_core().pimpl() == core.pimpl());
		CHECK(logger_mov.name() == fstlog::small_string<32>{"logger_1"});
		CHECK(logger_mov.channel() == 10);
		CHECK(logger_mov.thread() == fstlog::small_string<32>{"thread_1"});
		CHECK(logger_mov.level() == fstlog::level::Debug);
		CHECK(logger_mov.dropped() == dropped_logs);

		CHECK(logger_mov2.get_core().pimpl() == core.pimpl());
		CHECK(logger_mov2.name() == fstlog::small_string<32>{"logger_2"});
		CHECK(logger_mov2.channel() == 10);
		CHECK(logger_mov2.thread() == fstlog::small_string<32>{"thread_2"});
		CHECK(logger_mov2.level() == fstlog::level::Debug);
		CHECK(logger_mov2.dropped() == dropped_logs_2);
	};	
}
