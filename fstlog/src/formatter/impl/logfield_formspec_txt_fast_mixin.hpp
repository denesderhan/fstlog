//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <detail/unaligned_span.hpp>
#include <formatter/impl/detail/logfield.hpp>
#include <formatter/impl/detail/format_setting_txt_fast.hpp>
#include <formatter/impl/detail/format_str_helper.hpp>

namespace fstlog {
    template<typename L>
    class logfield_formspec_txt_fast_mixin : public L
    {
    public:
        using format_type = format_setting_txt_fast;

        static void set_format( 
            [[maybe_unused]] logfield field, 
            [[maybe_unused]] byte_span_const form_spec) noexcept {
        }

        static constexpr format_type get_format(
            [[maybe_unused]] logfield field) noexcept 
        {
            return format_type{};
        }

        static constexpr format_type get_default_format() noexcept {
            return format_type{};
        }

        static constexpr format_type get_format(
            byte_span_const form_spec) noexcept
        {
            format_type form;
            if (form_spec.empty()) return form;
            if (form_spec.size_bytes() == 1) {
                if (form_spec[0] >= 'A') form.type = form_spec[0];
                return form;
            }
            const auto* const begin = form_spec.data_bytes();
            const auto* const end = begin + form_spec.size_bytes();
            const auto* pos = end - 1;
            if(*pos >= 'A') form.type = *pos--;
            const auto* const num_end = pos + 1;
            while (*pos >= '0' && *pos <= '9' && pos > begin) pos--; // precision is second last in format str
            if (*pos != '.') return form; // no precision
            pos++;
            std::uint16_t precision = 0;
            while (pos < num_end && precision <= 999) {
                precision *= 10;
                precision += *pos++ - '0';
            }
            form.precision = precision;
            return form;
        }

        static constexpr format_type get_format_align_width(
            [[maybe_unused]] format_type format) noexcept
        {
            return format_type{};
        }
    };
}
