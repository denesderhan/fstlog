//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>
#include <string_view>
#include <utility>

#include <fstlog/compatible.hpp>
#include <fstlog/core.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/logger/detail/log/log_addmeta_mixin.hpp>
#include <fstlog/logger/detail/log/log_check_buffer_mixin.hpp>
#include <fstlog/logger/detail/log/log_check_core_mixin.hpp>
#include <fstlog/logger/detail/log/log_compute_msgsize_mixin.hpp>
#include <fstlog/logger/detail/log/log_level_filter_mixin.hpp>
#include <fstlog/logger/detail/log/log_policy_mixin.hpp>
#include <fstlog/logger/detail/logger_base_mixin.hpp>
#include <fstlog/logger/detail/logger_buffer_mixin.hpp>
#include <fstlog/logger/detail/logger_channel_mixin.hpp>
#include <fstlog/logger/detail/logger_core_mixin.hpp>
#include <fstlog/logger/detail/logger_dropcount_st_mixin.hpp>
#include <fstlog/logger/detail/logger_level_mixin.hpp>
#include <fstlog/logger/detail/logger_msgsize_mixin.hpp>
#include <fstlog/logger/detail/logger_name_mixin.hpp>
#include <fstlog/logger/detail/logger_thread_mixin.hpp>
#include <fstlog/logger/detail/logger_writer_mixin.hpp>
#include <fstlog/logger/detail/stamp_chrono_mixin.hpp>

namespace fstlog {
    
    using logger_st_impl =
        log_level_filter_mixin <
        log_check_core_mixin <
        log_check_buffer_mixin <
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
        >>>>>>>>>>>>>>>;

    // non thread safe logger
    class logger_st final 
        : private logger_st_impl
    {
    public:
        explicit logger_st(
            core core,
            std::string_view logger_name = "Unnamed",
            fstlog::level level = level::All,
            channel_type channel = constants::default_log_channel,
            std::string_view thread_name = "",
            std::uint32_t buffer_size = 0) noexcept(
                noexcept(handle_error(error_code::none)))
        {
            error_code error{ error_code::none };
            if (!compatible()) error = error_code::incomp_api;
            else if (!memory_resource_identical()) error = error_code::mem_res_bad;
            else {
                logger_st_impl::set_core(std::move(core));
                logger_st_impl::new_buffer(buffer_size);
                logger_st_impl::set_name(logger_name);
                logger_st_impl::set_channel(channel);
                logger_st_impl::set_level(level);
                if (!thread_name.empty()) {
                    logger_st_impl::set_thread(thread_name);
                }
            }
            handle_error(error);
        }

        template<
            fstlog::level level,
            template<class T> class policy,
            log_call_flag flags,
            class... Args>
        void log(Args const&... args) noexcept(
            noexcept(std::declval<logger_st_impl&>().template log<level, policy, flags>(args...)))
        {
            logger_st_impl::template log<level, policy, flags>(args...);
        }

        template<
            template<class T> class policy,
            log_call_flag flags,
            class... Args>
        void log(fstlog::level level, Args const&... args) noexcept(
            noexcept(std::declval<logger_st_impl&>().template log<policy, flags>(level, args...)))
        {
            logger_st_impl::template log<policy, flags>(level, args...);
        }

        void set_core(core core) noexcept {
            logger_st_impl::set_core(std::move(core));
        }
        
        core get_core() noexcept {
            return logger_st_impl::get_core();
        }
        
        small_string<32> name() const noexcept {
            return logger_st_impl::name();
        }
        
        void set_name(small_string<32> name) noexcept {
            logger_st_impl::set_name(name);
        }
        
        small_string<32> thread() const noexcept {
            return logger_st_impl::thread();
        }
        
        void set_thread(small_string<32> thread) noexcept {
            logger_st_impl::set_thread(thread);
        }
        
        channel_type channel() const noexcept {
            return logger_st_impl::channel();
        }
        
        void set_channel(channel_type channel) noexcept {
            logger_st_impl::set_channel(channel);
        }
        
        fstlog::level level() noexcept {
            return logger_st_impl::level();
        }
        
        void set_level(fstlog::level level) noexcept {
            logger_st_impl::set_level(level);
        }
        
        std::uintmax_t dropped() noexcept {
            return logger_st_impl::dropped();
        }
        
        void new_buffer() noexcept {
            logger_st_impl::new_buffer();
        }
        
        void new_buffer(std::uint32_t buffer_size) noexcept {
            logger_st_impl::new_buffer(buffer_size);
        }
        
        bool good() const noexcept {
            return is_core_set() && is_buffer_set();
        }
    };
}
