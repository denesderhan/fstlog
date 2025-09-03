//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstddef>

#include <detail/unaligned_span.hpp>
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/formatter/formatter.hpp>
#include <formatter/formatter_interface.hpp>

namespace fstlog {
    template<std::size_t fmt_buff_size, class L>
    class sink_formatter_mixin : public L {
    public:
        using memory_resource_type = typename L::memory_resource_type;

        explicit sink_formatter_mixin(memory_resource_type* resource) noexcept(
            noexcept(L(nullptr)))
            : L(resource) {}

        sink_formatter_mixin(const sink_formatter_mixin& other) = delete;
        sink_formatter_mixin(sink_formatter_mixin&& other) = delete;
        sink_formatter_mixin& operator=(const sink_formatter_mixin& rhs) = delete;
        sink_formatter_mixin& operator=(sink_formatter_mixin&& rhs) = delete;

        ~sink_formatter_mixin() noexcept {
            if (formatter_.pimpl() != nullptr) {
                formatter_.pimpl()->release();
            }
        }

        error_code set_formatter(formatter formatter) noexcept {
            if (!formatter.good()) return error_code::obj_null;
            if (!formatter.pimpl()->use()) return error_code::obj_locked;
            formatter_ = std::move(formatter);
            return error_code::none;
        }

        byte_span_const format(byte_span_const message) noexcept {
            FSTLOG_ASSERT(formatter_.pimpl() != nullptr);
            return formatter_.pimpl()->format_message(message, format_buffer_);
        }

    private:
        formatter formatter_;
        std::array<unsigned char, fmt_buff_size> format_buffer_{0};
    };
}
