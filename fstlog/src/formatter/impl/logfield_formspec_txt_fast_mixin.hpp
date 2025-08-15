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
        using allocator_type = typename L::allocator_type;
        typedef format_setting_txt_fast format_type;
    
        logfield_formspec_txt_fast_mixin() noexcept(
            noexcept(allocator_type())
            && noexcept(logfield_formspec_txt_fast_mixin(allocator_type{})))
            : logfield_formspec_txt_fast_mixin(allocator_type{}) {}
        explicit logfield_formspec_txt_fast_mixin(allocator_type const& allocator) noexcept(
            noexcept(L(allocator_type{})))
            : L(allocator) {}

        logfield_formspec_txt_fast_mixin(const logfield_formspec_txt_fast_mixin& other) noexcept(
            noexcept(logfield_formspec_txt_fast_mixin::get_allocator())
            && noexcept(logfield_formspec_txt_fast_mixin(
                logfield_formspec_txt_fast_mixin{}, allocator_type{})))
            : logfield_formspec_txt_fast_mixin(other, other.get_allocator()) {}
        logfield_formspec_txt_fast_mixin(const logfield_formspec_txt_fast_mixin& other, allocator_type const& allocator) noexcept(
            noexcept(L(logfield_formspec_txt_fast_mixin{}, allocator_type{})))
            : L(other, allocator) {}
        
        logfield_formspec_txt_fast_mixin(logfield_formspec_txt_fast_mixin&& other) = delete;
        logfield_formspec_txt_fast_mixin& operator=(const logfield_formspec_txt_fast_mixin& rhs) = delete;
        logfield_formspec_txt_fast_mixin& operator=(logfield_formspec_txt_fast_mixin&& rhs) = delete;
        
        ~logfield_formspec_txt_fast_mixin() = default;

        static void set_format( 
            [[maybe_unused]] logfield field, 
            [[maybe_unused]] byte_span_const form_spec) noexcept {}

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
            const auto begin = form_spec.data_bytes();
            const auto end = begin + form_spec.size_bytes();
            auto pos = end - 1;
            if(*pos >= 'A') form.type = *pos--;
            auto num_end = pos + 1;
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
