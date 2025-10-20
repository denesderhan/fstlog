//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once

#if !defined(FSTLOG_CLANG_NTTP) && defined(__clang__)
#if __has_extension(cxx_generalized_nttp)
#define FSTLOG_CLANG_NTTP
#endif
#endif
#if (defined(__cpp_nontype_template_args) && __cpp_nontype_template_args >= 201911L)\
    || (defined(__GNUC__) && defined(__cpp_nontype_template_parameter_class)\
        && __cpp_nontype_template_parameter_class >= 201806L)\
    || defined(FSTLOG_CLANG_NTTP)
#include <cstdint>
#include <utility>

#include <fstlog/compatible.hpp>
#include <fstlog/core.hpp>
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
    class logger_st_fix final 
        : private logger_st_fix_impl<logger_name_, level_, log_channel_, thread_name_>
    {
    public:
        explicit logger_st_fix(core core, std::uint32_t buffer_size = 0) noexcept(
                noexcept(handle_error(error_code::none)))
        {
            error_code error{ error_code::none };
            if (!compatible()) error = error_code::incomp_api;
            else if (!memory_resource_identical()) error = error_code::mem_res_bad;
            else {
                logger_st_fix_t::set_core(std::move(core));
                logger_st_fix_t::new_buffer(buffer_size);
            }
            handle_error(error);
        }
        
        using logger_st_fix_t = logger_st_fix_impl<logger_name_, level_, log_channel_, thread_name_>;

        template<
            fstlog::level level,
            template<class T> class policy,
            log_call_flag flags,
            class... Args>
        void log(Args const&... args) noexcept(
            noexcept(std::declval<logger_st_fix_t&>().template log<level, policy, flags>(args...)))
        {
            logger_st_fix_t::template log<level, policy, flags>(args...);
        }

        template<
            template<class T> class policy,
            log_call_flag flags,
            class... Args>
        void log(fstlog::level level, Args const&... args) noexcept(
            noexcept(std::declval<logger_st_fix_t&>().template log<policy, flags>(level, args...)))
        {
            logger_st_fix_t::template log<policy, flags>(level, args...);
        }

        void set_core(core core) noexcept {
            logger_st_fix_t::set_core(std::move(core));
        }

        core get_core() noexcept {
            return logger_st_fix_t::get_core();
        }

        auto name() const noexcept {
            return logger_st_fix_t::name();
        }

        auto thread() const noexcept {
            return logger_st_fix_t::thread();
        }

        channel_type channel() const noexcept (
            noexcept(logger_st_fix_t::channel()))
        {
            return logger_st_fix_t::channel();
        }

        fstlog::level level() noexcept {
            return logger_st_fix_t::level();
        }

        std::uintmax_t dropped() noexcept {
            return logger_st_fix_t::dropped();
        }

        void new_buffer() noexcept {
            logger_st_fix_t::new_buffer();
        }

        void new_buffer(std::uint32_t buffer_size) noexcept {
            logger_st_fix_t::new_buffer(buffer_size);
        }

        bool good() const noexcept {
            return this->is_core_set() && this->is_buffer_set();
        }
    };
}
#else
#error "fstlog::logger_st_fix missing C++ feature __cpp_nontype_template_args (available in C++20)"
#endif
