//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <output/output_interface.hpp>

#include <detail/unaligned_span.hpp>
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
    template<class L>
    class sink_output_mixin : public L {
    public:
        using memory_resource_type = typename L::memory_resource_type;

        explicit sink_output_mixin(memory_resource_type* resource) noexcept(
            noexcept(L(nullptr)))
            : L(resource) {}

        sink_output_mixin(const sink_output_mixin& other) = delete;
        sink_output_mixin(sink_output_mixin&& other) = delete;
        sink_output_mixin& operator=(const sink_output_mixin& rhs) = delete;
        sink_output_mixin& operator=(sink_output_mixin&& rhs) = delete;

        ~sink_output_mixin() noexcept {
            if (output_.pimpl() != nullptr) {
                output_.pimpl()->flush();
                output_.pimpl()->release();
            }
        }

        error_code set_output(output output) noexcept {
            if (!output.good()) return error_code::obj_null;
            if (!output.pimpl()->use()) return error_code::obj_locked;
            output_ = std::move(output);
            return error_code::none;
        }

        void write_message(byte_span_const message) noexcept {
            FSTLOG_ASSERT(output_.pimpl() != nullptr);
            output_.pimpl()->write_message(message);
        }

        void output_flush() noexcept {
            FSTLOG_ASSERT(output_.pimpl() != nullptr);
            output_.pimpl()->flush();
        }

    private:
        output output_;
    };
}
