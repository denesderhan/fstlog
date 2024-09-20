//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>
#include <string_view>

#include <fstlog/core.hpp>
#include <fstlog/detail/api_def.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/logger/detail/log/log_addmeta_mixin.hpp>
#include <fstlog/logger/detail/log/log_check_buffer_mixin.hpp>
#include <fstlog/logger/detail/log/log_check_core_mixin.hpp>
#include <fstlog/logger/detail/log/log_compute_msgsize_mixin.hpp>
#include <fstlog/logger/detail/log/log_level_filter_mixin.hpp>
#include <fstlog/logger/detail/log/log_policy_mixin.hpp>
#include <fstlog/logger/detail/log/log_try_get_buffer_mixin.hpp>
#include <fstlog/logger/detail/logger_base_mixin.hpp>
#include <fstlog/logger/detail/logger_buffer_thread_local_mixin.hpp>
#include <fstlog/logger/detail/logger_channel_mixin.hpp>
#include <fstlog/logger/detail/logger_core_mixin.hpp>
#include <fstlog/logger/detail/logger_dropcount_mixin.hpp>
#include <fstlog/logger/detail/logger_level_atomic_mixin.hpp>
#include <fstlog/logger/detail/logger_msgsize_thread_local_mixin.hpp>
#include <fstlog/logger/detail/logger_name_mixin.hpp>
#include <fstlog/logger/detail/logger_thread_thread_local_mixin.hpp>
#include <fstlog/logger/detail/logger_writer_mixin.hpp>
#include <fstlog/logger/detail/stamp_chrono_mixin.hpp>

namespace fstlog {
	using logger_impl =
		log_level_filter_mixin <
		log_check_core_mixin <
		log_try_get_buffer_mixin <
		log_check_buffer_mixin <
		log_addmeta_mixin <
		log_compute_msgsize_mixin <
		log_policy_mixin <
		logger_writer_mixin <
		logger_buffer_thread_local_mixin <
		logger_core_mixin <
		logger_msgsize_thread_local_mixin <
		logger_name_mixin <
		logger_thread_thread_local_mixin <
		logger_channel_mixin <
		logger_level_atomic_mixin <
		stamp_chrono_mixin <
		logger_dropcount_mixin<
		logger_base_mixin
		>>>>>>>>>>>>>>>>>;
	
	// thread safe logger 
	// Calling logger.log() can throw if buffer allocation fails. 
	// If new_buffer() is called (per thread) and good() returns true,
	// allocation succeeded and no exceptions will be thrown (or asserts triggered).
	// If good() returns false logging can be disabled with set_level()
	class alignas(constants::cache_ls_nosharing) logger final 
		: private logger_impl
	{
	public:
		logger() noexcept = default;

		explicit logger(
			core core,
			std::string_view name = "Unnamed",
			fstlog::level level = level::All,
			channel_type channel = constants::default_log_channel ) noexcept
		{
			logger_impl::set_core(std::move(core));
			logger_impl::set_name(name);
			logger_impl::set_level(level);
			logger_impl::set_channel(channel);
		}

		~logger() noexcept = default;

		logger(const logger& other) noexcept
			: logger_impl(other) {}

		logger(logger&& other) noexcept
			: logger_impl(std::move(other)){}
		
		// not thread safe
		logger& operator=(const logger& other) noexcept {
			logger_impl::operator=(other);
			return *this;
		}
		// not thread safe
		logger& operator=(logger&& other) noexcept {
			logger_impl::operator=(std::move(other));
			return *this;
		}
		// thread safe
		template<
			fstlog::level level,
			template<class T> class policy,
			log_call_flag flags,
			class... Args>
		void log(Args const&... args) noexcept(
			noexcept(logger_impl{}.template log<level, policy, flags>(args...)))
		{
			logger_impl::template log<level, policy, flags>(args...);
		}
		// thread safe
		template<
			template<class T> class policy,
			log_call_flag flags,
			class... Args>
		void log(fstlog::level level, Args const&... args) noexcept(
			noexcept(logger_impl{}.template log<policy, flags>(level, args...)))
		{
			logger_impl::template log<policy, flags>(level, args...);
		}
		// not thread safe
		void set_core(core core) noexcept(
			noexcept(logger_impl::set_core(fstlog::core{ nullptr })))
		{
			logger_impl::set_core(std::move(core));
		}
		// thread safe
		core get_core() noexcept(
			noexcept(logger_impl::get_core()))
		{
			return logger_impl::get_core();
		}
		// thread safe
		small_string<32> name() const noexcept(
			noexcept(logger_impl::name())) 
		{
			return logger_impl::name();
		}
		// not thread safe
		void set_name(small_string<32> name) noexcept (
			noexcept(logger_impl::set_name(small_string<32>{})))
		{
			logger_impl::set_name(name);
		}
		// thread safe
		static small_string<32> thread() noexcept(
			noexcept(logger_impl::thread()))
		{
			return logger_impl::thread();
		}
		// thread safe
		static void set_thread(small_string<32> thread) noexcept (
			noexcept(logger_impl::set_thread(small_string<32>{})))
		{
			logger_impl::set_thread(thread);
		}
		// thread safe
		channel_type channel() const noexcept (
			noexcept(logger_impl::channel()))
		{
			return logger_impl::channel();
		}
		// not thread safe
		void set_channel(channel_type channel) noexcept (
			noexcept(logger_impl::set_channel(channel_type{})))
		{
			logger_impl::set_channel(channel);
		}
		// thread safe
		fstlog::level level() noexcept (
			noexcept(logger_impl::level()))
		{
			return logger_impl::level();
		}
		// thread safe
		void set_level(fstlog::level level) noexcept (
			noexcept(logger_impl::set_level(fstlog::level{})))
		{
			logger_impl::set_level(level);
		}
		// thread safe
		std::uintmax_t dropped() noexcept(
			noexcept(logger_impl::dropped()))
		{
			return logger_impl::dropped();
		}
		// thread safe, all loggers with the same core in the same thread
		// share the same buffer, this will affect all of them.  
		static void new_buffer(core& core) noexcept {
			logger_impl::new_buffer(core);
		}
		static void new_buffer(core&& core) noexcept {
			logger_impl::new_buffer(core);
		}
		// thread safe, all loggers with the same core in the same thread
		// share the same buffer, this will affect all of them. 
		static void new_buffer(core& core, std::uint32_t buffer_size) noexcept {
			logger_impl::new_buffer(core, buffer_size);
		}
		static void new_buffer(core&& core, std::uint32_t buffer_size) noexcept {
			logger_impl::new_buffer(core, buffer_size);
		}
		// thread safe, all loggers with the same core in the same thread
		// share the same buffer, this will affect all of them.  
		static void release_buffer(const core& core) noexcept {
			logger_impl::release_buffer(core);
		}
		// thread safe
		bool good() const noexcept {
			return is_core_set() && is_buffer_set();
		}
	};
}
