//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstddef>
#include <cstring>
#include <type_traits>

#include <detail/unaligned_span.hpp>
#include <fstlog/detail/ut_cast.hpp>
#include <formatter/impl/detail/logfield.hpp>
#include <formatter/impl/detail/format_setting_txt.hpp>
#include <formatter/impl/detail/format_str_helper.hpp>

namespace fstlog {
    template<typename L>
    class logfield_formspec_txt_mixin : public L
    {
    public:
        using memory_resource_type = typename L::memory_resource_type;
        typedef format_setting_txt format_type;

        explicit logfield_formspec_txt_mixin(memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<L, memory_resource_type*>)
            : L(resource)
        {
            field_formattings_.fill(get_default_format()); //noexcept
        }

        logfield_formspec_txt_mixin(const logfield_formspec_txt_mixin& other) noexcept(
            noexcept(other.get_memory_resource())
            && std::is_nothrow_constructible_v<
                logfield_formspec_txt_mixin,
                const logfield_formspec_txt_mixin&,
                memory_resource_type*>)
            : logfield_formspec_txt_mixin(other, other.get_memory_resource()) {}
        logfield_formspec_txt_mixin(const logfield_formspec_txt_mixin& other, memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<
                L,
                const L&,
                memory_resource_type*>)
            : L(static_cast<const L&>(other), resource),
            field_formattings_{ other.field_formattings_ } {} //noexcept
        
        logfield_formspec_txt_mixin(logfield_formspec_txt_mixin&& other) = delete;
        logfield_formspec_txt_mixin& operator=(const logfield_formspec_txt_mixin& rhs) = delete;
        logfield_formspec_txt_mixin& operator=(logfield_formspec_txt_mixin&& rhs) = delete;
        
        ~logfield_formspec_txt_mixin() = default;
        
        //form_spec is without curly brackets and ":", ({name:form_spec})
        void set_format(
            logfield field, 
            byte_span_const form_spec) noexcept 
        {
            FSTLOG_ASSERT(field < logfield_last);
            field_formattings_[ut_cast(field)] = get_format(form_spec);
        }

        format_type get_format(logfield field) const noexcept {
            FSTLOG_ASSERT(field < logfield_last);
            return field_formattings_[ut_cast(field)];
        }

        static constexpr format_type get_default_format() noexcept {
            return format_type{};
        }

        static constexpr format_type get_format(
            byte_span_const form_spec) noexcept 
        {
            format_type out;
            if (form_spec.empty()) return out;
            // fill align
            auto align_pos = form_spec.data_bytes();
            form_spec = skip_fill_align(form_spec);
            auto align_end = form_spec.data_bytes();
            // sign
            if (form_spec.empty()) return out;
            auto fmt_c = form_spec.template get<0>();
            if (fmt_c == '-' || fmt_c == ' ' || fmt_c == '+') {
                out.sign = fmt_c;
                form_spec.template drop_front<1>();
                if (form_spec.empty()) return out;
                fmt_c = form_spec.template get<0>();
            }
            // alternate
            if (fmt_c == '#') {
                out.alternate = true;
                form_spec.template drop_front<1>();
                if (form_spec.empty()) return out;
                fmt_c = form_spec.template get<0>();
            }
            //skip ["0"]
            if (fmt_c == '0') {
                form_spec.template drop_front<1>();
                if (form_spec.empty()) return out;
            }
            // get width
            out.width = get_width(form_spec);
            // set fill align
            if (out.width != 0 && align_pos < align_end) {
                out.align = *--align_end;
                const std::size_t utf8_seq_len = static_cast<std::size_t>(align_end - align_pos);
                FSTLOG_ASSERT(utf8_seq_len <= out.fill_char.size());
                std::memcpy(out.fill_char.data(), align_pos, utf8_seq_len);
            }
            if (form_spec.empty()) return out;
            
            // precision
            out.precision = get_precision(form_spec);
            if (form_spec.empty()) return out;
            
            // type
            out.type = *(form_spec.data_bytes() + form_spec.size() - 1);
            
            return out;
        }

        static constexpr format_type get_format_align_width(
            format_type format) noexcept
        {
            format_type out{};
            out.align = format.align;
            out.fill_char = format.fill_char;
            out.width = format.width;
            return out;
        }

    private:
        std::array<format_type, ut_cast(logfield_last)> field_formattings_;
    };
}
