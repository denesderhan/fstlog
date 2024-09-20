//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <detail/byte_span.hpp>
#include <formatter/impl/detail/logfield.hpp>
#include <formatter/impl/detail/format_setting_txt_fast.hpp>
#include <formatter/impl/detail/uint_fromchars_4digit.hpp>

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
            [[maybe_unused]] buff_span_const form_spec) noexcept {}

        static constexpr format_type get_format(
			[[maybe_unused]] logfield field) noexcept 
		{
            return format_type{};
        }

        static constexpr format_type get_default_format() noexcept {
            return format_type{};
        }

        static constexpr format_type get_format(
			buff_span_const form_spec) noexcept
		{
            format_type form;
            auto pos{ form_spec.data() };
            const auto end{ form_spec.data() + form_spec.size_bytes() };
            if (pos == end) return form;

            auto last_char = *(end - 1);
            if ((last_char >= 'A') && (last_char != 'L') && (last_char != '^'))
                form.type = last_char;
            
            //skip '.' if part of fill-align
            if (end - pos >= 2){
                const auto mb_al{ *(pos + 1) };
                if ((mb_al == '<') || (mb_al == '>') || (mb_al == '^'))
                    pos += 2;
            }
                        
            while (pos < end && *pos != '.') pos++;
            pos++;
			uint_fromchars_4digit(form.precision, pos, end);
			return form;
        }

		static constexpr format_type get_format_align_width(
			[[maybe_unused]] format_type format) noexcept
		{
			return format_type{};
		}
    };
}
