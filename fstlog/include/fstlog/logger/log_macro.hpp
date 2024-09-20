//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#ifndef FSTLOG_COMPILETIME_LOGLEVEL
#define FSTLOG_COMPILETIME_LOGLEVEL All
#endif
#include <fstlog/logger/log_policy_guaranteed.hpp>
#include <fstlog/logger/log_policy_lowlatency.hpp>
#include <fstlog/logger/log_policy_nonguaranteed.hpp>

#undef LOG
#define LOG(logger, level, ...) \
	logger.template log<fstlog::log_policy_guaranteed, fstlog::ut_cast(fstlog::log_metaargs::All)>(\
		level, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_TRACE
#define LOG_TRACE(logger, ...) if constexpr (ut_cast(fstlog::level::Trace) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Trace, fstlog::log_policy_guaranteed, \
		fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_DEBUG
#define LOG_DEBUG(logger, ...) if constexpr (ut_cast(fstlog::level::Debug) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Debug, fstlog::log_policy_guaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_INFO
#define LOG_INFO(logger, ...) if constexpr (ut_cast(fstlog::level::Info) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Info, fstlog::log_policy_guaranteed, \
fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_WARN
#define LOG_WARN(logger, ...) if constexpr (ut_cast(fstlog::level::Warn) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Warn, fstlog::log_policy_guaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_ERROR
#define LOG_ERROR(logger, ...) if constexpr (ut_cast(fstlog::level::Error) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Error, fstlog::log_policy_guaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_FATAL
#define LOG_FATAL(logger, ...) if constexpr (ut_cast(fstlog::level::Fatal) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Fatal, fstlog::log_policy_guaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_LL
#define LOG_LL(logger, level, ...) \
	logger.template log<fstlog::log_policy_lowlatency, fstlog::ut_cast(fstlog::log_metaargs::All)>(\
		level, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_LL_TRACE
#define LOG_LL_TRACE(logger, ...) if constexpr (ut_cast(fstlog::level::Trace) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Trace, fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_LL_DEBUG
#define LOG_LL_DEBUG(logger, ...) if constexpr (ut_cast(fstlog::level::Debug) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Debug, fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_LL_INFO
#define LOG_LL_INFO(logger, ...) if constexpr (ut_cast(fstlog::level::Info) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Info, fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_LL_WARN
#define LOG_LL_WARN(logger, ...) if constexpr (ut_cast(fstlog::level::Warn) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Warn, fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_LL_ERROR
#define LOG_LL_ERROR(logger, ...) if constexpr (ut_cast(fstlog::level::Error) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Error, fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_LL_FATAL
#define LOG_LL_FATAL(logger, ...) if constexpr (ut_cast(fstlog::level::Fatal) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Fatal, fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_NG
#define LOG_NG(logger, level, ...) \
	logger.template log<fstlog::log_policy_nonguaranteed, fstlog::ut_cast(fstlog::log_metaargs::All)>(\
		level, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_NG_TRACE
#define LOG_NG_TRACE(logger, ...) if constexpr (ut_cast(fstlog::level::Trace) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Trace, fstlog::log_policy_nonguaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_NG_DEBUG
#define LOG_NG_DEBUG(logger, ...) if constexpr (ut_cast(fstlog::level::Debug) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Debug, fstlog::log_policy_nonguaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_NG_INFO
#define LOG_NG_INFO(logger, ...) if constexpr (ut_cast(fstlog::level::Info) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Info, fstlog::log_policy_nonguaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_NG_WARN
#define LOG_NG_WARN(logger, ...) if constexpr (ut_cast(fstlog::level::Warn) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Warn, fstlog::log_policy_nonguaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_NG_ERROR
#define LOG_NG_ERROR(logger, ...) if constexpr (ut_cast(fstlog::level::Error) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Error, fstlog::log_policy_nonguaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_NG_FATAL
#define LOG_NG_FATAL(logger, ...) if constexpr (ut_cast(fstlog::level::Fatal) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Fatal, fstlog::log_policy_nonguaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
