//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <mutex>

#include <fstlog/detail/constants.hpp>
#include <fstlog/logger/detail/log/log_addmeta_mixin.hpp>
#include <fstlog/logger/detail/log/log_nobuffer_nolog_mixin.hpp>
#include <fstlog/logger/detail/log/log_compute_msgsize_mixin.hpp>
#include <fstlog/logger/detail/logger_dropcount_mixin.hpp>
#include <fstlog/logger/detail/log/log_level_filter_mixin.hpp>
#include <fstlog/logger/detail/logger_msgsize_mixin.hpp>
#include <fstlog/logger/detail/log/log_policy_fix_mixin.hpp>
#include <fstlog/logger/detail/logger_writer_mixin.hpp>
#include <fstlog/logger/detail/logger_base_mixin.hpp>
#include <fstlog/logger/detail/logger_buffer_mixin.hpp>
#include <fstlog/logger/detail/logger_channel_compile_time_mixin.hpp>
#include <fstlog/logger/detail/logger_level_compile_time_mixin.hpp>
#include <fstlog/logger/detail/logger_thread_query_mixin.hpp>
#include <fstlog/logger/detail/stamp_chrono_mixin.hpp>
#include <fstlog/logger/log_policy_lowlatency.hpp>

namespace fstlog {
	template<class L>
	class logger_name_background : public L {
	public:
		static constexpr std::string_view name() noexcept {
			return "fstlog_self";
		}
	};

	using  logger_background_impl =
		log_nobuffer_nolog_mixin <
		log_level_filter_mixin <
		log_addmeta_mixin <
		log_compute_msgsize_mixin <
		log_policy_fix_mixin < log_policy_lowlatency,
		logger_writer_mixin <
		logger_buffer_mixin <
		logger_msgsize_mixin <
		logger_name_background <
		logger_thread_query_mixin <
		logger_channel_compile_time_mixin <constants::self_log_channel,
		logger_level_compile_time_mixin <level::All,
		stamp_chrono_mixin <
		logger_dropcount_mixin<
		logger_base_mixin
		>>>>>>>>>>>>>>;

	class alignas(constants::cache_ls_nosharing) logger_background final 
		: private logger_background_impl
	{
	public:
		logger_background() noexcept = default;
		~logger_background() noexcept = default;
		logger_background(const logger_background& other) = delete;
		logger_background(logger_background&& other) = delete;
		logger_background& operator=(const logger_background& other) = delete;
		logger_background& operator=(logger_background&& other) = delete;

		//not thread safe
		void init(log_buffer buffer) noexcept {
			// if buffer allocation fails (nullptr) log() will do nothing
			// see: log_nobuffer_nolog_mixin
			set_buffer(std::move(buffer));
		}
		//thread safe
		template<
			fstlog::level level,
			template<class T> class policy,
			log_call_flag flags,
			class... Args>
		void log(Args const&... args) noexcept(
			noexcept(logger_background_impl{}.template log<level, policy, flags>(args...)))
		{
			std::lock_guard grd{ logger_mutex_ };
			logger_background_impl::template log<level, policy, flags>(args...);
		}
		//thread safe
		template<
			template<class T> class policy,
			log_call_flag flags,
			class... Args>
		void log(fstlog::level level, Args const&... args) noexcept(
			noexcept(logger_background_impl{}.template log<policy, flags>(level, args...)))
		{
			std::lock_guard grd{ logger_mutex_ };
			logger_background_impl::template log<policy, flags>(level, args...);
		}
		//thread safe
		small_string<32> name() const noexcept(
			noexcept(logger_background_impl::name()))
		{
			return logger_background_impl::name();
		}
		//thread safe
		std::uint32_t thread() const noexcept(
			noexcept(logger_background_impl::thread()))
		{
			return logger_background_impl::thread();
		}
		//thread safe
		channel_type channel() const noexcept (
			noexcept(logger_background_impl::channel()))
		{
			return logger_background_impl::channel();
		}
		//thread safe
		fstlog::level level() noexcept (
			noexcept(logger_background_impl::level()))
		{
			return logger_background_impl::level();
		}
		//thread safe
		std::uintmax_t dropped() noexcept(
			noexcept(logger_background_impl::dropped()))
		{
			return logger_background_impl::dropped();
		}
	private:
		std::mutex logger_mutex_;
	};
}
