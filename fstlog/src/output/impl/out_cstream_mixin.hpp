//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdio>
#include <type_traits>

#include <detail/unaligned_span.hpp>
#include <fstlog/detail/error_code.hpp>

namespace fstlog {
    template<class L>
    class out_cstream_mixin : public L
    {
    public:
        out_cstream_mixin() noexcept = default;

        out_cstream_mixin(const out_cstream_mixin&) = delete;
        out_cstream_mixin& operator=(const out_cstream_mixin&) = delete;
        out_cstream_mixin(out_cstream_mixin&&) = delete;
        out_cstream_mixin& operator=(out_cstream_mixin&&) = delete;

        ~out_cstream_mixin() noexcept {
            if (stream_ != nullptr) {
                fflush(stream_);
            }
        }

        error_code set_stream(FILE* stream_ptr) noexcept {
            if (stream_ != nullptr) return error_code::double_init;
            if (stream_ptr == nullptr) return error_code::stream_bad;
            stream_ = stream_ptr;
            return error_code::none;
        }

        void write_message(byte_span_const msg) noexcept {
            FSTLOG_ASSERT(stream_ != nullptr);
            FSTLOG_ASSERT(msg.data_bytes() != nullptr);
            fwrite(
                msg.data_bytes(),
                sizeof(char),
                msg.size_bytes(),
                stream_);
        }
        void flush() noexcept {
            FSTLOG_ASSERT(stream_ != nullptr);
            fflush(stream_);
        }
    private:
        FILE* stream_{ nullptr };
    };
}
