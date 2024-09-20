//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#ifndef FSTLOG_COMPILETIME_LOGLEVEL
#define FSTLOG_COMPILETIME_LOGLEVEL All
#endif
#include <fstlog/logger/log_policy_guaranteed.hpp>
#include <fstlog/logger/log_policy_lowlatency.hpp>
#include <fstlog/logger/log_policy_nonguaranteed.hpp>

#undef LOGN
#define LOGN(logger, level, ...) \
	logger.template log<fstlog::log_policy_guaranteed, fstlog::ut_cast(fstlog::log_metaargs::None)>(\
		level, __VA_ARGS__)

#undef LOGN_TRACE
#define LOGN_TRACE(logger, ...) if constexpr (ut_cast(fstlog::level::Trace) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Trace, fstlog::log_policy_guaranteed, \
		fstlog::ut_cast(fstlog::log_metaargs::None)>(__VA_ARGS__)

#undef LOGN_DEBUG
#define LOGN_DEBUG(logger, ...) if constexpr (ut_cast(fstlog::level::Debug) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Debug, fstlog::log_policy_guaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::None)>(__VA_ARGS__)

#undef LOGN_INFO
#define LOGN_INFO(logger, ...) if constexpr (ut_cast(fstlog::level::Info) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Info, fstlog::log_policy_guaranteed, \
fstlog::ut_cast(fstlog::log_metaargs::None)>(__VA_ARGS__)

#undef LOGN_WARN
#define LOGN_WARN(logger, ...) if constexpr (ut_cast(fstlog::level::Warn) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Warn, fstlog::log_policy_guaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::None)>(__VA_ARGS__)

#undef LOGN_ERROR
#define LOGN_ERROR(logger, ...) if constexpr (ut_cast(fstlog::level::Error) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Error, fstlog::log_policy_guaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::None)>(__VA_ARGS__)

#undef LOGN_FATAL
#define LOGN_FATAL(logger, ...) if constexpr (ut_cast(fstlog::level::Fatal) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Fatal, fstlog::log_policy_guaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::None)>(__VA_ARGS__)

#undef LOGN_LL
#define LOGN_LL(logger, level, ...) \
	logger.template log<fstlog::log_policy_lowlatency, fstlog::ut_cast(fstlog::log_metaargs::None)>(\
		level, __VA_ARGS__)

#undef LOGN_LL_TRACE
#define LOGN_LL_TRACE(logger, ...) if constexpr (ut_cast(fstlog::level::Trace) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Trace, fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::None)>(__VA_ARGS__)

#undef LOGN_LL_DEBUG
#define LOGN_LL_DEBUG(logger, ...) if constexpr (ut_cast(fstlog::level::Debug) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Debug, fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::None)>(__VA_ARGS__)

#undef LOGN_LL_INFO
#define LOGN_LL_INFO(logger, ...) if constexpr (ut_cast(fstlog::level::Info) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Info, fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::None)>(__VA_ARGS__)

#undef LOGN_LL_WARN
#define LOGN_LL_WARN(logger, ...) if constexpr (ut_cast(fstlog::level::Warn) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Warn, fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::None)>(__VA_ARGS__)

#undef LOGN_LL_ERROR
#define LOGN_LL_ERROR(logger, ...) if constexpr (ut_cast(fstlog::level::Error) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Error, fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::None)>(__VA_ARGS__)

#undef LOGN_LL_FATAL
#define LOGN_LL_FATAL(logger, ...) if constexpr (ut_cast(fstlog::level::Fatal) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Fatal, fstlog::log_policy_lowlatency, \
	fstlog::ut_cast(fstlog::log_metaargs::None)>(__VA_ARGS__)

#undef LOGN_NG
#define LOGN_NG(logger, level, ...) \
	logger.template log<fstlog::log_policy_nonguaranteed, fstlog::ut_cast(fstlog::log_metaargs::None)>(\
		level, __VA_ARGS__)

#undef LOGN_NG_TRACE
#define LOGN_NG_TRACE(logger, ...) if constexpr (ut_cast(fstlog::level::Trace) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Trace, fstlog::log_policy_nonguaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::None)>(__VA_ARGS__)

#undef LOGN_NG_DEBUG
#define LOGN_NG_DEBUG(logger, ...) if constexpr (ut_cast(fstlog::level::Debug) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Debug, fstlog::log_policy_nonguaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::None)>(__VA_ARGS__)

#undef LOGN_NG_INFO
#define LOGN_NG_INFO(logger, ...) if constexpr (ut_cast(fstlog::level::Info) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Info, fstlog::log_policy_nonguaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::None)>(__VA_ARGS__)

#undef LOGN_NG_WARN
#define LOGN_NG_WARN(logger, ...) if constexpr (ut_cast(fstlog::level::Warn) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Warn, fstlog::log_policy_nonguaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::None)>(__VA_ARGS__)

#undef LOGN_NG_ERROR
#define LOGN_NG_ERROR(logger, ...) if constexpr (ut_cast(fstlog::level::Error) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Error, fstlog::log_policy_nonguaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::None)>(__VA_ARGS__)

#undef LOGN_NG_FATAL
#define LOGN_NG_FATAL(logger, ...) if constexpr (ut_cast(fstlog::level::Fatal) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) \
	logger.template log<fstlog::level::Fatal, fstlog::log_policy_nonguaranteed, \
	fstlog::ut_cast(fstlog::log_metaargs::None)>(__VA_ARGS__)
