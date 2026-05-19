//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#ifndef FSTLOG_COMPILETIME_LOGLEVEL
#define FSTLOG_COMPILETIME_LOGLEVEL All
#endif
#include <fstlog/logger/log_policy_guaranteed.hpp>

#include <source_location>
#include <string_view>

template<template<class T> class Policy, 
    typename Logger, 
    fstlog::level compt_level = fstlog::level::All>
struct LogProxy {
    Logger& logger;
    std::source_location loc;
    fstlog::level level{ fstlog::level::All };
    
    template<typename... Args>
    void operator()(Args&&... args) const {
        if constexpr (ut_cast(compt_level) == ut_cast(fstlog::level::All)) {
            logger.template log<Policy,
                fstlog::ut_cast(fstlog::log_metaargs::All)>(
                    level,
                    std::string_view{ loc.file_name() },
                    loc.line(),
                    std::string_view{ loc.function_name() },
                    std::forward<Args>(args)...);
        }
        else if constexpr (ut_cast(compt_level) <= ut_cast(fstlog::level::FSTLOG_COMPILETIME_LOGLEVEL)) {
            logger.template log<compt_level, Policy,
                fstlog::ut_cast(fstlog::log_metaargs::All)>(
                    std::string_view{ loc.file_name() },
                    loc.line(),
                    std::string_view{ loc.function_name() },
                    std::forward<Args>(args)...);
        }
    }
};

template<template<class T> class Policy = fstlog::log_policy_guaranteed,
    typename Logger>
LogProxy<Policy, Logger> log(
    Logger& logger, 
    fstlog::level level,
    std::source_location loc = std::source_location::current())
{
    return { logger, loc, level };
}

template<template<class T> class Policy = fstlog::log_policy_guaranteed,
    typename Logger>
LogProxy<Policy, Logger, fstlog::level::Trace> log_trace(Logger& logger,
    std::source_location loc = std::source_location::current()) {
    return { logger, loc };
}

template<template<class T> class Policy = fstlog::log_policy_guaranteed,
    typename Logger>
LogProxy<Policy, Logger, fstlog::level::Debug> log_debug(Logger& logger,
    std::source_location loc = std::source_location::current()) {
    return { logger, loc };
}

template<template<class T> class Policy = fstlog::log_policy_guaranteed,
    typename Logger>
LogProxy<Policy, Logger, fstlog::level::Info> log_info(Logger& logger,
    std::source_location loc = std::source_location::current()) {
    return { logger, loc };
}

template<template<class T> class Policy = fstlog::log_policy_guaranteed,
    typename Logger>
LogProxy<Policy, Logger, fstlog::level::Warn> log_warn(Logger& logger,
    std::source_location loc = std::source_location::current()) {
    return { logger, loc };
}

template<template<class T> class Policy = fstlog::log_policy_guaranteed,
    typename Logger>
LogProxy<Policy, Logger, fstlog::level::Error> log_error(Logger& logger,
    std::source_location loc = std::source_location::current()) {
    return { logger, loc };
}

template<template<class T> class Policy = fstlog::log_policy_guaranteed,
    typename Logger>
LogProxy<Policy, Logger, fstlog::level::Fatal> log_fatal(Logger& logger,
    std::source_location loc = std::source_location::current()) {
    return { logger, loc };
}
