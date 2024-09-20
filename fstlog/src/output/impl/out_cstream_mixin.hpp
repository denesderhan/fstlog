//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <stdio.h>

#include <detail/byte_span.hpp>
#include <detail/safe_reinterpret_cast.hpp>

namespace fstlog {
    template<class L>
    class out_cstream_mixin : public L
    {
    public:
        using allocator_type = typename L::allocator_type;

		out_cstream_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(out_cstream_mixin(allocator_type{})))
			: out_cstream_mixin(allocator_type{}) {}
		explicit out_cstream_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
            : L(allocator) {}

        out_cstream_mixin(const out_cstream_mixin& other) = delete;
        out_cstream_mixin(out_cstream_mixin&& other) = delete;
        out_cstream_mixin& operator=(const out_cstream_mixin& rhs) = delete;
        out_cstream_mixin& operator=(out_cstream_mixin&& rhs) = delete;

        ~out_cstream_mixin() noexcept {
            if (stream_ != nullptr) {
				fflush(stream_);
            }
        }

        const char* set_stream(FILE* stream_ptr) noexcept {
			if (stream_ptr == nullptr) return "Stream was nullptr!";
			stream_ = stream_ptr;
			return nullptr;
        }

        void write_message(buff_span_const msg) noexcept {
            FSTLOG_ASSERT(stream_ != nullptr);
			FSTLOG_ASSERT(msg.data() != nullptr);
            fwrite(
                msg.data(),
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
