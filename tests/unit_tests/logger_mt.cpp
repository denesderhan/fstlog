//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <fstlog/core.hpp>
#include <fstlog/logger/log_macro.hpp>
#include <fstlog/logger/logger_mt.hpp>

//for each SECTION the TEST_CASE is executed from the start! (preventing multiple core constructs)
inline fstlog::core core;

TEST_CASE("logger_mt") {
	core.poll_interval(std::chrono::milliseconds{0});

	SECTION("construct") {
		fstlog::logger_mt logger;
		CHECK(logger.get_core().pimpl() == nullptr);
		CHECK(logger.name() == fstlog::small_string<32>{"Unnamed"});
		CHECK(logger.channel() == 1);
		CHECK(logger.thread() == fstlog::this_thread::get_id());
		CHECK(logger.dropped() == 0);
		CHECK(logger.level() == fstlog::level::All);
		
		CHECK(logger.good() == false);
		logger.set_core(core);
		CHECK(logger.get_core().pimpl() == core.pimpl());
		CHECK(logger.good() == false);
		logger.new_buffer(1024);
		CHECK(logger.good() == true);

		logger.set_name("logger_1");
		logger.set_level(fstlog::level::Debug);
		logger.set_channel(10);
		CHECK(logger.name() == fstlog::small_string<32>{"logger_1"});
		CHECK(logger.channel() == 10);
		CHECK(logger.level() == fstlog::level::Debug);

		for (int i = 0; i < 10; i++) {
			LOG_LL_FATAL(logger, "Test text, test text, test text, test text, test text, test text, test text.");
		}
		CHECK(logger.dropped() > 0);

		//copy construct/assign
		fstlog::logger_mt logger_2(core, "logger_2", fstlog::level::Debug, 10, 1024);
				
		CHECK(logger_2.get_core().pimpl() == core.pimpl());
		CHECK(logger_2.name() == fstlog::small_string<32>{"logger_2"});
		CHECK(logger_2.channel() == 10);
		CHECK(logger_2.level() == fstlog::level::Debug);
		CHECK(logger_2.dropped() == 0);

		for (int i = 0; i < 10; i++) {
			LOG_LL_FATAL(logger_2, "Test text, test text, test text, test text, test text, test text, test text.");
		}
		CHECK(logger_2.dropped() > 0);
	};
}
