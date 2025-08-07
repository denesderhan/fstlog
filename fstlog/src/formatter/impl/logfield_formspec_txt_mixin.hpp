//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstddef>

#include <detail/safe_reinterpret_cast.hpp>
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
        using allocator_type = typename L::allocator_type;
        typedef format_setting_txt format_type;

        logfield_formspec_txt_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(logfield_formspec_txt_mixin(allocator_type{})))
			: logfield_formspec_txt_mixin(allocator_type{}) {}
        explicit logfield_formspec_txt_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
			: L(allocator)
		{
            field_formattings_.fill(get_default_format()); //noexcept
        }

		logfield_formspec_txt_mixin(const logfield_formspec_txt_mixin& other) noexcept(
			noexcept(logfield_formspec_txt_mixin::get_allocator())
			&& noexcept(logfield_formspec_txt_mixin(logfield_formspec_txt_mixin{}, allocator_type{})))
            : logfield_formspec_txt_mixin(other, other.get_allocator()) {}
		logfield_formspec_txt_mixin(const logfield_formspec_txt_mixin& other, allocator_type const& allocator) noexcept(
			noexcept(L(logfield_formspec_txt_mixin{}, allocator_type{})))
            : L(other, allocator),
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
			auto pos = form_spec.data();
			const auto end = pos + form_spec.size_bytes();
			// fill align
			auto align_pos = pos;
			pos = skip_fill_align(pos, end);
			auto align_end = pos;
			if (pos == end) return out;
			// sign
			if (*pos == '-' || *pos == ' ' || *pos == '+') {
				out.sign = *pos++;
				if (pos == end) return out;
			}
			// alternate
			if (*pos == '#') {
				out.alternate = true;
				pos++;
				if (pos == end) return out;
			}
			//skip ["0"]
			if (*pos == '0') {
				pos++;
				if (pos == end) return out;
			}
			// get width
            out.width = static_cast<std::uint16_t>(get_width(pos, end));
			// set fill align
			if (out.width != 0 && align_pos < align_end) {
				out.align = *--align_end;
				FSTLOG_ASSERT(align_end - align_pos <= 4);
				memcpy(out.fill_char.data(), align_pos, static_cast<std::size_t>(align_end - align_pos));
            }
			if (pos == end) return out;
			
			// precision
            int precision = static_cast<int>(out.precision);
            get_precision(precision, pos, end);
            out.precision = static_cast<std::uint16_t>(precision);
			if (pos == end) return out;
			
			// type
			out.type = *(end - 1);
			
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
