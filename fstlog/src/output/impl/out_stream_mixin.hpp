//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <ostream>

#include <detail/byte_span.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
    template<class L>
    class out_stream_mixin : public L
    {
    public:
        using allocator_type = typename L::allocator_type;

        out_stream_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(out_stream_mixin(allocator_type{})))
			: out_stream_mixin(allocator_type{}) {}
		explicit out_stream_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
            : L(allocator) {}

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

        const char* set_stream(std::shared_ptr<std::ostream> stream_smart_ptr) noexcept {
			if (stream_smart_ptr == nullptr || !stream_smart_ptr->good()) {
				return "Stream was bad/nullptr!";
			}
			//this can not throw, stream was good() (no error state)
			stream_smart_ptr->exceptions(std::ios_base::iostate(0));
			stream_ = std::move(stream_smart_ptr);
			return nullptr;
        }

        void write_message(buff_span_const msg) noexcept {
            FSTLOG_ASSERT(stream_ != nullptr);
			FSTLOG_ASSERT(msg.data() != nullptr);
            //can not throw (stream_ptr_->exceptions(0))
            stream_->write(
                safe_reinterpret_cast<const char*>(msg.data()),
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
