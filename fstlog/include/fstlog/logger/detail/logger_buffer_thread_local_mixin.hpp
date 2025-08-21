//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <limits>
#include <cstdint>

#include <fstlog/core.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/log_buffer.hpp>

namespace fstlog {
    template<class L>
    class logger_buffer_thread_local_mixin : public L {
    public:
        logger_buffer_thread_local_mixin() noexcept = default;
        ~logger_buffer_thread_local_mixin() noexcept = default;

        logger_buffer_thread_local_mixin(const logger_buffer_thread_local_mixin& other) noexcept = default;
        logger_buffer_thread_local_mixin& operator=(const logger_buffer_thread_local_mixin& other) noexcept = default;

        logger_buffer_thread_local_mixin(logger_buffer_thread_local_mixin&& other) noexcept
            : L(std::move(other)) {}
                
        logger_buffer_thread_local_mixin& operator=(logger_buffer_thread_local_mixin&& other) noexcept {
            L::operator=(std::move(other));
            return *this;
        }

        // this affects all loggers attached to this logger's core, 
        // in the thread this method is called
        void new_buffer() noexcept(noexcept(buffer().size()))
        {
            // we can call is_buffer_set() and set_buffer() only if core is set
            if (this->is_core_set()) {
                std::uint32_t buffer_size{ 0 };
                if (is_buffer_set()) { 
                    buffer_size = buffer().size();
                }
                set_buffer(this->get_buffer(buffer_size));
            }
        }

        // this affects all loggers attached to this logger's core, 
        // in the thread this method is called
        void new_buffer(std::uint32_t buffer_size) noexcept {
            if (this->is_core_set()) {
                set_buffer(this->get_buffer(buffer_size));
            }
        }

        // this affects all loggers attached to this logger's core, 
        // in the thread this method is called
        void release_buffer() noexcept {
            if (this->is_core_set()) {
                buffer() = log_buffer{};
            }
        }

        bool is_buffer_set() noexcept {
            return buffer().good();
        }

        log_buffer& buffer() noexcept {
            FSTLOG_ASSERT(this->is_core_set());
            return this->core_.detail_tls_buffer();
        }

        // this affects all loggers attached to this core, logging in this thread
        void set_buffer(log_buffer new_buffer) noexcept {
            buffer() = std::move(new_buffer);
        }

    };
}
