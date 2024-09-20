//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#if __cpp_nontype_template_args == 201911L
#define FSTLOG_TEST_FIX_LOGGER
#include <fstlog/logger/logger_st_fix.hpp>
#endif

#include <fstlog/detail/noexceptions.hpp>
#include <fstlog/logger/logger_mt.hpp>
#include <fstlog/logger/logger_st.hpp>
#include <fstlog/logger/logger.hpp>


#include <fstlog/logger/log_policy_guaranteed.hpp>
#include <fstlog/logger/log_policy_nonguaranteed.hpp>
#include <fstlog/logger/log_policy_lowlatency.hpp>

namespace {
	template<class L>
	class fake_policy_true : public L {
	public:
		template<fstlog::level level, fstlog::log_call_flag flags, typename... Args>
		void log(Args const&... args) noexcept {}

		template<fstlog::log_call_flag flags, typename... Args>
		void log(fstlog::level level, Args const&... args) noexcept {}
	};
	template<class L>
	class fake_policy_false : public L {
	public:
		template<fstlog::level level, fstlog::log_call_flag flags, typename... Args>
		void log(Args const&... args) {}

		template<fstlog::log_call_flag flags, typename... Args>
		void log(fstlog::level level, Args const&... args) {}
	};
}

namespace fstlog {
	class alignas(constants::cache_ls_nosharing) logger_test final : public
		log_level_filter_mixin <
		log_addmeta_mixin <
		log_compute_msgsize_mixin <
		log_policy_mixin <
		logger_writer_mixin <
		logger_buffer_mixin <
		logger_core_mixin <
		logger_msgsize_mixin <
		logger_name_mixin <
		logger_thread_mixin <
		logger_channel_mixin <
		logger_level_mixin <
		stamp_chrono_mixin <
		logger_dropcount_st_mixin<
		logger_base_mixin >
		>>>>>>>>>>>>>
	{
	public:
		explicit logger_test(
			core core,
			std::string_view logger_name = "",
			fstlog::level level = level::Trace,
			std::string_view thread_name = "",
			channel_type channel = constants::default_log_channel,
			std::uint32_t buffer_size = 0) noexcept
		{
			set_core(std::move(core));
			new_buffer(buffer_size);
			set_name(logger_name);
			set_channel(channel);
			set_level(level);
			if (thread_name.empty()) {
				set_thread(this_thread::get_str());
			}
			else {
				set_thread(thread_name);
			}
		}
		bool good() const noexcept {
			return is_core_set() && is_buffer_set();
		}
	};
}

#ifdef FSTLOG_NOEXCEPTIONS
static constexpr bool noexceptions{ true };
#else
static constexpr bool noexceptions{ false };
#endif

TEST_CASE("log_noexcept") {
	SECTION("logger") {
		fstlog::logger l{ fstlog::core{nullptr} };
		CHECK(noexcept(l.log<fstlog::level::Info, fstlog::log_policy_guaranteed, 0>(0)) == noexceptions);
		CHECK(noexcept(l.log<fstlog::level::Info, fstlog::log_policy_nonguaranteed, 0>(0)) == noexceptions);
		CHECK(noexcept(l.log<fstlog::level::Info, fstlog::log_policy_lowlatency, 0>(0)) == noexceptions);
		CHECK(noexcept(l.log<fstlog::level::Info, fake_policy_false, 0>(0)) == false);
		CHECK(noexcept(l.log<fstlog::level::Info, fake_policy_true, 0>(0)) == noexceptions);
	};
	SECTION("logger_mt") {
		fstlog::logger_mt l{ fstlog::core{nullptr} };
		CHECK(noexcept(l.log<fstlog::level::Info, fstlog::log_policy_guaranteed, 0>(0)) == noexceptions);
		CHECK(noexcept(l.log<fstlog::level::Info, fstlog::log_policy_nonguaranteed, 0>(0)) == noexceptions);
		CHECK(noexcept(l.log<fstlog::level::Info, fstlog::log_policy_lowlatency, 0>(0)) == noexceptions);
		CHECK(noexcept(l.log<fstlog::level::Info, fake_policy_false, 0>(0)) == false);
		CHECK(noexcept(l.log<fstlog::level::Info, fake_policy_true, 0>(0)) == noexceptions);
	};
	SECTION("logger_st") {
		fstlog::logger_st l{ fstlog::core{nullptr} };
		CHECK(noexcept(l.log<fstlog::level::Info, fstlog::log_policy_guaranteed, 0>(0)) == noexceptions);
		CHECK(noexcept(l.log<fstlog::level::Info, fstlog::log_policy_nonguaranteed, 0>(0)) == noexceptions);
		CHECK(noexcept(l.log<fstlog::level::Info, fstlog::log_policy_lowlatency, 0>(0)) == noexceptions);
		CHECK(noexcept(l.log<fstlog::level::Info, fake_policy_false, 0>(0)) == false);
		CHECK(noexcept(l.log<fstlog::level::Info, fake_policy_true, 0>(0)) == noexceptions);
	};
#ifdef FSTLOG_TEST_FIX_LOGGER
	SECTION("logger_st_fix") {
		fstlog::logger_st_fix<> l{ fstlog::core{nullptr} };
		CHECK(noexcept(l.log<fstlog::level::Info, fstlog::log_policy_guaranteed, 0>(0)) == noexceptions);
		CHECK(noexcept(l.log<fstlog::level::Info, fstlog::log_policy_nonguaranteed, 0>(0)) == noexceptions);
		CHECK(noexcept(l.log<fstlog::level::Info, fstlog::log_policy_lowlatency, 0>(0)) == noexceptions);
		CHECK(noexcept(l.log<fstlog::level::Info, fake_policy_false, 0>(0)) == false);
		CHECK(noexcept(l.log<fstlog::level::Info, fake_policy_true, 0>(0)) == noexceptions);
	};
#endif
	SECTION("logger_test") {
		fstlog::logger_test l{ fstlog::core{nullptr} };
		CHECK(noexcept(l.log<fstlog::level::Info, fstlog::log_policy_guaranteed, 0>(0)) == true);
		CHECK(noexcept(l.log<fstlog::level::Info, fstlog::log_policy_nonguaranteed, 0>(0)) == true);
		CHECK(noexcept(l.log<fstlog::level::Info, fstlog::log_policy_lowlatency, 0>(0)) == true);
		CHECK(noexcept(l.log<fstlog::level::Info, fake_policy_false, 0>(0)) == false);
		CHECK(noexcept(l.log<fstlog::level::Info, fake_policy_true, 0>(0)) == true);
	};
}
