//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <fstlog/core.hpp>
#include <fstlog/logger/log_macro.hpp>
#include <fstlog/logger/logger.hpp>

//for each SECTION the TEST_CASE is executed from the start! (preventing multiple core constructs)
inline fstlog::core core;

TEST_CASE("logger") {
	core.poll_interval(std::chrono::milliseconds{0});

	SECTION("construct") {
		fstlog::logger logger;
		CHECK(logger.get_core().pimpl() == nullptr);
		CHECK(logger.name() == fstlog::small_string<32>{"Unnamed"});
		CHECK(logger.channel() == 1);
		CHECK(logger.thread() == fstlog::this_thread::get_str());
		CHECK(logger.dropped() == 0);
		CHECK(logger.level() == fstlog::level::All);
		
		CHECK(logger.good() == false);
		logger.set_core(core);
		CHECK(logger.get_core().pimpl() == core.pimpl());
		CHECK(logger.good() == false);
		logger.new_buffer(core, 1024);
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
		fstlog::logger logger_cpy{logger};
		fstlog::logger logger_cpy2;
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
		CHECK(logger_cpy.thread() == fstlog::small_string<32>{"thread_1"});
		CHECK(logger_cpy.level() == fstlog::level::Debug);
		CHECK(logger_cpy.dropped() == 0);

		CHECK(logger_cpy2.get_core().pimpl() == core.pimpl());
		CHECK(logger_cpy2.name() == fstlog::small_string<32>{"Unnamed"});
		CHECK(logger_cpy2.channel() == 10);
		CHECK(logger_cpy2.thread() == fstlog::small_string<32>{"thread_1"});
		CHECK(logger_cpy2.level() == fstlog::level::Debug);
		CHECK(logger_cpy2.dropped() == 0);

		logger_cpy.set_name("logger_2");
		CHECK(logger_cpy.name() == fstlog::small_string<32>{"logger_2"});
		for (int i = 0; i < 10; i++) {
			LOG_LL_FATAL(logger_cpy, "Test text, test text, test text, test text, test text, test text, test text.");
		}
		CHECK(logger_cpy.dropped() > 0);

		const auto dropped_logs_2{ logger_cpy.dropped() };

		//move construct/assign
		fstlog::logger logger_mov{ std::move(logger) };
		fstlog::logger logger_mov2;
		logger_mov2 = std::move(logger_cpy);

		CHECK(logger.get_core().pimpl() == nullptr);
		CHECK(logger.name() == fstlog::small_string<32>{"Unnamed"});
		CHECK(logger.channel() == 10);
		CHECK(logger.thread() == fstlog::small_string<32>{"thread_1"});
		CHECK(logger.level() == fstlog::level::Debug);
		CHECK(logger.dropped() == 0);
		CHECK(logger_cpy.get_core().pimpl() == nullptr);
		CHECK(logger_cpy.name() == fstlog::small_string<32>{"Unnamed"});
		CHECK(logger_cpy.channel() == 10);
		CHECK(logger_cpy.thread() == fstlog::small_string<32>{"thread_1"});
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
		CHECK(logger_mov2.thread() == fstlog::small_string<32>{"thread_1"});
		CHECK(logger_mov2.level() == fstlog::level::Debug);
		CHECK(logger_mov2.dropped() == dropped_logs_2);

	};

	SECTION("multi_core") {
		fstlog::core temp_core0;
		auto last_id{ temp_core0.id() };
		for (auto i = last_id + 1; i <= 32; i++) {
			fstlog::core temp_core;
			CHECK(temp_core.id() == i);
			fstlog::logger temp_logger(temp_core);
			if (i != 32) {
				CHECK(temp_logger.get_core().pimpl() == temp_core.pimpl());
				CHECK(temp_logger.good() == false);
				temp_logger.new_buffer(temp_core, 1024);
				CHECK(temp_logger.good() == true);
			}
			else {
				CHECK(temp_logger.get_core().pimpl() == nullptr);
				CHECK(temp_logger.good() == false);
			}
		}
	};
	
}
