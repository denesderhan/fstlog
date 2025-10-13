//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <iostream>
#include <type_traits>

#include <detail/safe_reinterpret_cast.hpp>
#include <detail/unaligned_span.hpp>
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
    template<class L>
    class out_console_mixin : public L
    {
    public:
        using memory_resource_type = typename L::memory_resource_type;

        explicit out_console_mixin(memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<L, memory_resource_type*>)
            : L(resource) {}

        out_console_mixin(const out_console_mixin& other) = delete;
        out_console_mixin(out_console_mixin&& other) = delete;
        out_console_mixin& operator=(const out_console_mixin& rhs) = delete;
        out_console_mixin& operator=(out_console_mixin&& rhs) = delete;

        ~out_console_mixin() noexcept {
            flush();
        }
                
        error_code set_stream(std::ostream* stream_ptr) noexcept {
            // if already initialized
            if (stream_ptr_ != nullptr) {
                return error_code::double_init;
            }
            // only global, standard cout cerr clog is usable
            if (stream_ptr != &std::cout 
                && stream_ptr != &std::cerr 
                && stream_ptr != &std::clog)
            {
                return error_code::input_bad;
            }
            // if std::cout/cerr/clog was nullptr or bad
            if (stream_ptr == nullptr || !stream_ptr->good()) {
                return error_code::stream_bad;
            }
            //this can not throw, stream was good() (no error state)
            stream_ptr->exceptions(std::ios_base::iostate(0));
            stream_ptr_ = stream_ptr;
            return error_code::none;
        }

        void write_message(byte_span_const msg) noexcept {
            FSTLOG_ASSERT(stream_ptr_ != nullptr);
            FSTLOG_ASSERT(msg.data_bytes() != nullptr);
            const char* data = safe_reinterpret_cast<const char*>(msg.data_bytes());
            std::size_t bytes = msg.size_bytes();
            //can not throw (stream_ptr_->exceptions(0))
            stream_ptr_->write(data, bytes);
        }

        void flush() noexcept {
            if (stream_ptr_ != nullptr) {
                //can not throw (stream_.exceptions(0))
                stream_ptr_->flush();
            }
        }
    private:
        std::ostream* stream_ptr_{ nullptr };
    };
}
