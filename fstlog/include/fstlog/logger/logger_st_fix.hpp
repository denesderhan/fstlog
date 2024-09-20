//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once

#if defined(__cpp_nontype_template_args) && __cpp_nontype_template_args >= 201911L

#include <cstdint>

#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/small_string.hpp>
#include <fstlog/logger/detail/log/log_addmeta_mixin.hpp>
#include <fstlog/logger/detail/log/log_check_buffer_mixin.hpp>
#include <fstlog/logger/detail/log/log_check_core_mixin.hpp>
#include <fstlog/logger/detail/log/log_compute_msgsize_mixin.hpp>
#include <fstlog/logger/detail/log/log_level_filter_mixin.hpp>
#include <fstlog/logger/detail/log/log_policy_mixin.hpp>
#include <fstlog/logger/detail/logger_base_mixin.hpp>
#include <fstlog/logger/detail/logger_buffer_mixin.hpp>
#include <fstlog/logger/detail/logger_channel_compile_time_mixin.hpp>
#include <fstlog/logger/detail/logger_core_mixin.hpp>
#include <fstlog/logger/detail/logger_dropcount_st_mixin.hpp>
#include <fstlog/logger/detail/logger_level_compile_time_mixin.hpp>
#include <fstlog/logger/detail/logger_msgsize_mixin.hpp>
#include <fstlog/logger/detail/logger_name_compile_time_mixin.hpp>
#include <fstlog/logger/detail/logger_thread_compile_time_mixin.hpp>
#include <fstlog/logger/detail/logger_writer_mixin.hpp>
#include <fstlog/logger/detail/stamp_chrono_mixin.hpp>

namespace fstlog {
	template<small_string<16> logger_name_,
		fstlog::level level_,
		channel_type log_channel_,
		small_string<16> thread_name_>
	using logger_st_fix_impl =
		log_level_filter_mixin <
		log_check_core_mixin <
		log_check_buffer_mixin <
		log_addmeta_mixin <
		log_compute_msgsize_mixin <
		log_policy_mixin <
		logger_writer_mixin <
		logger_buffer_mixin <
		logger_core_mixin<
		logger_msgsize_mixin <
		logger_name_compile_time_mixin <logger_name_,
		logger_thread_compile_time_mixin <thread_name_,
		logger_channel_compile_time_mixin <log_channel_,
		logger_level_compile_time_mixin <level_,
		stamp_chrono_mixin <
		logger_dropcount_st_mixin<
		logger_base_mixin>
		>>>>>>>>>>>>>>>;

	// non thread safe logger
	template<auto logger_name_ = small_string<16>("Unnamed"),
		fstlog::level level_ = level::All,
		channel_type log_channel_ = constants::default_log_channel,
		auto thread_name_ = small_string<16>("Unnamed")>
	class alignas(constants::cache_ls_nosharing) logger_st_fix final 
		: private logger_st_fix_impl<logger_name_, level_, log_channel_, thread_name_>
	{
	public:
		logger_st_fix() noexcept = default;

		explicit logger_st_fix(core core, std::uint32_t buffer_size = 0)  noexcept {
			logger_st_fix_t::set_core(std::move(core));
			logger_st_fix_t::new_buffer(buffer_size);
		}

		~logger_st_fix() noexcept = default;

		using logger_st_fix_t = logger_st_fix_impl<logger_name_, level_, log_channel_, thread_name_>;

		logger_st_fix(const logger_st_fix& other) noexcept
			: logger_st_fix_t(other) {}

		logger_st_fix(logger_st_fix&& other) noexcept
			: logger_st_fix_t(std::move(other)) {}

		logger_st_fix& operator=(const logger_st_fix& other) noexcept {
			logger_st_fix_t::operator=(other);
			return *this;
		}

		logger_st_fix& operator=(logger_st_fix&& other) noexcept {
			logger_st_fix_t::operator=(std::move(other));
			return *this;
		}

		template<
			fstlog::level level,
			template<class T> class policy,
			log_call_flag flags,
			class... Args>
		void log(Args const&... args) noexcept(
			noexcept(logger_st_fix_t{}.template log<level, policy, flags>(args...)))
		{
			logger_st_fix_t::template log<level, policy, flags>(args...);
		}

		template<
			template<class T> class policy,
			log_call_flag flags,
			class... Args>
		void log(fstlog::level level, Args const&... args) noexcept(
			noexcept(logger_st_fix_t{}.template log<policy, flags>(level, args...)))
		{
			logger_st_fix_t::template log<policy, flags>(level, args...);
		}

		void set_core(core core) noexcept(
			noexcept(logger_st_fix_t::set_core(fstlog::core{ nullptr })))
		{
			logger_st_fix_t::set_core(std::move(core));
		}

		core get_core() noexcept(
			noexcept(logger_st_fix_t::get_core()))
		{
			return logger_st_fix_t::get_core();
		}

		small_string<32> name() const noexcept(
			noexcept(logger_st_fix_t::name()))
		{
			return logger_st_fix_t::name();
		}

		small_string<32> thread() const noexcept(
			noexcept(logger_st_fix_t::thread()))
		{
			return logger_st_fix_t::thread();
		}

		channel_type channel() const noexcept (
			noexcept(logger_st_fix_t::channel()))
		{
			return logger_st_fix_t::channel();
		}

		fstlog::level level() noexcept (
			noexcept(logger_st_fix_t::level()))
		{
			return logger_st_fix_t::level();
		}

		std::uintmax_t dropped() noexcept(
			noexcept(logger_st_fix_t::dropped()))
		{
			return logger_st_fix_t::dropped();
		}

		void new_buffer() noexcept(
			noexcept(logger_st_fix_t::new_buffer()))
		{
			logger_st_fix_t::new_buffer();
		}

		void new_buffer(std::uint32_t buffer_size) noexcept(
			noexcept(logger_st_fix_t::new_buffer(std::uint32_t{})))
		{
			logger_st_fix_t::new_buffer(buffer_size);
		}

		bool good() const noexcept {
			return this->is_core_set() && this->is_buffer_set();
		}
	};
}
#else
#error "fstlog::logger_st_fix missing C++ feature __cpp_nontype_template_args >= 201911L (available in C++20)"
#endif
