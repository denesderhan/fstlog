//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#define FSTLOG_COMPILETIME_LOGLEVEL Warn
#include <fstlog/logger/detail/log/log_level_filter_mixin.hpp>
#include <fstlog/detail/types.hpp>
#include <fstlog/detail/level.hpp>

namespace {
	template<typename T>
	class pol {};

	class mock_tester {
	public:
		template<
			fstlog::level level,
			template<class T> class policy,
			fstlog::log_call_flag flags,
			typename... Args>
		constexpr void log([[maybe_unused]] Args const&... args) {
			passed_ = true;
		}

		template<
			template<class T> class policy,
			fstlog::log_call_flag flags,
			class... Args>
		void log([[maybe_unused]] fstlog::level level, [[maybe_unused]] Args const&... args) {
			passed_ = true;
		}

		void set_level(fstlog::level level) {
			level_ = level;
		}

		fstlog::level level() const {
			return level_;
		}

		bool passed_through() {
			bool temp{ passed_ };
			passed_ = false;
			return temp;
		}

		bool passed_{ false };
		fstlog::level level_{fstlog::level::All};
	};

	using logger_type = fstlog::log_level_filter_mixin<mock_tester>;
}

TEST_CASE("log_level_filter_mixin") {

	SECTION("compile_time") {
		logger_type logr;
		logr.set_level(fstlog::level::All);

		logr.log<fstlog::level::Fatal, pol, 0>("Hi");
		CHECK(logr.passed_through() == true);
		logr.log<fstlog::level::Error, pol, 0>("Hi");
		CHECK(logr.passed_through() == true);
		logr.log<fstlog::level::Warn, pol, 0>("Hi");
		CHECK(logr.passed_through() == true);
		logr.log<fstlog::level::Info, pol, 0>("Hi");
		CHECK(logr.passed_through() == false);
		logr.log<fstlog::level::Debug, pol, 0>("Hi");
		CHECK(logr.passed_through() == false);
		logr.log<fstlog::level::Trace, pol, 0>("Hi");
		CHECK(logr.passed_through() == false);

		logr.log<pol, 0>(fstlog::level::Fatal, "Hi");
		CHECK(logr.passed_through() == true);
		logr.log<pol, 0>(fstlog::level::Error, "Hi");
		CHECK(logr.passed_through() == true);
		logr.log<pol, 0>(fstlog::level::Warn, "Hi");
		CHECK(logr.passed_through() == true);
		logr.log<pol, 0>(fstlog::level::Info, "Hi");
		CHECK(logr.passed_through() == false);
		logr.log<pol, 0>(fstlog::level::Debug, "Hi");
		CHECK(logr.passed_through() == false);
		logr.log<pol, 0>(fstlog::level::Trace, "Hi");
		CHECK(logr.passed_through() == false);
	};

	SECTION("run_time") {
		logger_type logr;
		logr.set_level(fstlog::level::Error);

		logr.log<fstlog::level::Fatal, pol, 0>("Hi");
		CHECK(logr.passed_through() == true);
		logr.log<fstlog::level::Error, pol, 0>("Hi");
		CHECK(logr.passed_through() == true);
		logr.log<fstlog::level::Warn, pol, 0>("Hi");
		CHECK(logr.passed_through() == false);
		logr.log<fstlog::level::Info, pol, 0>("Hi");
		CHECK(logr.passed_through() == false);
		logr.log<fstlog::level::Debug, pol, 0>("Hi");
		CHECK(logr.passed_through() == false);
		logr.log<fstlog::level::Trace, pol, 0>("Hi");
		CHECK(logr.passed_through() == false);

		logr.log<pol, 0>(fstlog::level::Fatal, "Hi");
		CHECK(logr.passed_through() == true);
		logr.log<pol, 0>(fstlog::level::Error, "Hi");
		CHECK(logr.passed_through() == true);
		logr.log<pol, 0>(fstlog::level::Warn, "Hi");
		CHECK(logr.passed_through() == false);
		logr.log<pol, 0>(fstlog::level::Info, "Hi");
		CHECK(logr.passed_through() == false);
		logr.log<pol, 0>(fstlog::level::Debug, "Hi");
		CHECK(logr.passed_through() == false);
		logr.log<pol, 0>(fstlog::level::Trace, "Hi");
		CHECK(logr.passed_through() == false);

		logr.set_level(fstlog::level::None);

		logr.log<fstlog::level::Fatal, pol, 0>("Hi");
		CHECK(logr.passed_through() == false);
		logr.log<fstlog::level::Error, pol, 0>("Hi");
		CHECK(logr.passed_through() == false);
		logr.log<fstlog::level::Warn, pol, 0>("Hi");
		CHECK(logr.passed_through() == false);
		logr.log<fstlog::level::Info, pol, 0>("Hi");
		CHECK(logr.passed_through() == false);
		logr.log<fstlog::level::Debug, pol, 0>("Hi");
		CHECK(logr.passed_through() == false);
		logr.log<fstlog::level::Trace, pol, 0>("Hi");
		CHECK(logr.passed_through() == false);

		logr.log<pol, 0>(fstlog::level::Fatal, "Hi");
		CHECK(logr.passed_through() == false);
		logr.log<pol, 0>(fstlog::level::Error, "Hi");
		CHECK(logr.passed_through() == false);
		logr.log<pol, 0>(fstlog::level::Warn, "Hi");
		CHECK(logr.passed_through() == false);
		logr.log<pol, 0>(fstlog::level::Info, "Hi");
		CHECK(logr.passed_through() == false);
		logr.log<pol, 0>(fstlog::level::Debug, "Hi");
		CHECK(logr.passed_through() == false);
		logr.log<pol, 0>(fstlog::level::Trace, "Hi");
		CHECK(logr.passed_through() == false);
	};
}
