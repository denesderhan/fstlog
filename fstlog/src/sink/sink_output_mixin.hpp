//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <output/output_interface.hpp>

#include <detail/byte_span.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
    template<class L>
    class sink_output_mixin : public L {
    public:
        using allocator_type = typename L::allocator_type;

        sink_output_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(sink_output_mixin(allocator_type{})))
			: sink_output_mixin(allocator_type{}) {}
		explicit sink_output_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
            : L(allocator) {}

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

		const char* set_output(output output) noexcept {
			if (!output.good()) return "Output was bad!";
			if (!output.pimpl()->use()) return "Output used by another sink!";
			output_ = std::move(output);
			return nullptr;
		}

        void write_message(buff_span_const message) noexcept {
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
