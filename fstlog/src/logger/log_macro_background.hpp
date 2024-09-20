//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#ifndef FSTLOG_COMPILETIME_LOGLEVEL
#define FSTLOG_COMPILETIME_LOGLEVEL All
#endif
#include <fstlog/logger/log_policy_lowlatency.hpp>

#undef LOG_LL
#define LOG_LL(logger, level, ...) \
	logger.log<fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(level, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_LL_TRACE
#define LOG_LL_TRACE(logger, ...) if constexpr (ut_cast(fstlog::level::Trace) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.log<fstlog::level::Trace, fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_LL_DEBUG
#define LOG_LL_DEBUG(logger, ...) if constexpr (ut_cast(fstlog::level::Debug) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.log<fstlog::level::Debug, fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_LL_INFO
#define LOG_LL_INFO(logger, ...) if constexpr (ut_cast(fstlog::level::Info) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.log<fstlog::level::Info, fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_LL_WARN
#define LOG_LL_WARN(logger, ...) if constexpr (ut_cast(fstlog::level::Warn) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.log<fstlog::level::Warn, fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_LL_ERROR
#define LOG_LL_ERROR(logger, ...) if constexpr (ut_cast(fstlog::level::Error) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.log<fstlog::level::Error, fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#undef LOG_LL_FATAL
#define LOG_LL_FATAL(logger, ...) if constexpr (ut_cast(fstlog::level::Fatal) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.log<fstlog::level::Fatal, fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::All)>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
