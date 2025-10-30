//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstddef>
#include <cstring>
#include <type_traits>

#include <detail/unaligned_span.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <fstlog/detail/memory_resource.hpp>
#include <fstlog/detail/small_string.hpp>
#include <fstlog/detail/ut_cast.hpp>
#include <formatter/impl/detail/format_str_helper.hpp>
#include <formatter/impl/detail/logfield.hpp>

namespace fstlog {
    template<typename L>
    class logfield_formspec_fmt_mixin : public L
    {
    public:
        typedef small_string<24> format_type;

        logfield_formspec_fmt_mixin() noexcept {
            field_formattings_.fill(get_default_format()); //noexcept
        }

        logfield_formspec_fmt_mixin(const logfield_formspec_fmt_mixin& other) noexcept(
            noexcept(other.get_memory_resource())
            && std::is_nothrow_constructible_v<
                logfield_formspec_fmt_mixin,
                const logfield_formspec_fmt_mixin&,
                memory_resource*>)
            : logfield_formspec_fmt_mixin(other, other.get_memory_resource()) {}
        logfield_formspec_fmt_mixin(const logfield_formspec_fmt_mixin& other, memory_resource* resource) noexcept(
            std::is_nothrow_constructible_v<
                L,
                const L&,
                memory_resource*>)
            : L(static_cast<const L&>(other), resource),
            field_formattings_{ other.field_formattings_ } {} //noexcept
        
        logfield_formspec_fmt_mixin(logfield_formspec_fmt_mixin&& other) = delete;
        logfield_formspec_fmt_mixin& operator=(const logfield_formspec_fmt_mixin& rhs) = delete;
        logfield_formspec_fmt_mixin& operator=(logfield_formspec_fmt_mixin&& rhs) = delete;
        
        ~logfield_formspec_fmt_mixin() = default;
        
    public:
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
            return format_type{ std::string_view{"{}"} };
        }

        static constexpr format_type get_format(
            byte_span_const form_spec) noexcept 
        {
            const std::size_t form_spec_size = form_spec.size_bytes();
            if (form_spec.empty() || form_spec_size > 20)
                return get_default_format();
            std::array<char, 24> temp_str{"{:}}}}}}}}}}}}}}}}}}}}}"};
            std::memcpy(&temp_str[2], form_spec.data_bytes(), form_spec_size);
            return format_type{ std::string_view{ &temp_str[0], form_spec_size + 3 } };
        }

        static constexpr format_type get_format_align_width(
            format_type format, unsigned char type_char = 0) noexcept 
        {
            std::array<unsigned char, 24> out{ "{:}}}}}}}}}}}}}}}}}}}}}" };
            // empty format spec
            if (format.size() <= 3) {
                if (type_char == 0) {
                    return get_default_format();
                }
                else {
                    out[2] = type_char;
                    return format_type{ safe_reinterpret_cast<const char*>(out.data()),  4 };
                }
            }
            byte_span_const input(
                safe_reinterpret_cast<const unsigned char*>(format.data()) + 2,
                format.size() - 3);
            byte_span output(out.data() + 2, out.size() - 3); // 21 bytes
            // copy fill-align
            auto temp = input;
            input = skip_fill_align(input);
            const auto align_len = temp.size() - input.size();
            FSTLOG_ASSERT(align_len <= 5);
            std::memcpy(output.data_bytes(), temp.data_bytes(), align_len);
            output.drop_front(align_len); // max 5 byte
            // skip
            input = skip_sign_alt_0(input);
            // copy width
            temp = input;
            input = skip_valid_fmt_number(input);
            const auto width_len = static_cast<std::size_t>(temp.size() - input.size());
            std::memcpy(output.data_bytes(), temp.data_bytes(), width_len);
            output.drop_front(width_len); // max 4 bytes

            if (type_char != 0) {
                output.template set<0>(type_char);
                output.template drop_front<1>();
            }
            return format_type{ 
                safe_reinterpret_cast<const char*>(out.data()),  
                static_cast<std::size_t>(output.data_bytes() - out.data()) + 1};
        }

    private:
        std::array<format_type, ut_cast(logfield_last)> field_formattings_;
    };
}
