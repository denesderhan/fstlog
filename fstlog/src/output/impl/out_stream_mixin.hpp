//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <ostream>
#include <type_traits>

#include <detail/unaligned_span.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/error_code.hpp>

namespace fstlog {
    template<class L>
    class out_stream_mixin : public L
    {
    public:
        out_stream_mixin() noexcept = default;

        out_stream_mixin(const out_stream_mixin& other) = delete;
        out_stream_mixin(out_stream_mixin&& other) = delete;
        out_stream_mixin& operator=(const out_stream_mixin& rhs) = delete;
        out_stream_mixin& operator=(out_stream_mixin&& rhs) = delete;

        ~out_stream_mixin() noexcept {
            if (stream_ != nullptr) {
                //can not throw (stream_ptr_->exceptions(0))
                stream_->flush();
            }
        }

        error_code set_stream(std::shared_ptr<std::ostream> stream_smart_ptr) noexcept {
            if (stream_ != nullptr) {
                return error_code::double_init;
            }
            if (stream_smart_ptr == nullptr ){
                return error_code::obj_null;
            }
            if (!stream_smart_ptr->good()) {
                return error_code::stream_bad;
            }
            //this can not throw, stream was good() (no error state)
            stream_smart_ptr->exceptions(std::ios_base::iostate(0));
            stream_ = std::move(stream_smart_ptr);
            return error_code::none;
        }

        void write_message(byte_span_const msg) noexcept {
            FSTLOG_ASSERT(stream_ != nullptr);
            FSTLOG_ASSERT(msg.data_bytes() != nullptr);
            //can not throw (stream_ptr_->exceptions(0))
            stream_->write(
                safe_reinterpret_cast<const char*>(msg.data_bytes()),
                msg.size_bytes());
        }
        void flush() noexcept {
            FSTLOG_ASSERT(stream_ != nullptr);
            //can not throw (stream_ptr_->exceptions(0))
            stream_->flush();
        }
    private:
        std::shared_ptr<std::ostream> stream_;
    };
}
