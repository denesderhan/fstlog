//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstddef>
#include <cstring>
#pragma intrinsic(memcpy)


#include <detail/safe_reinterpret_cast.hpp>
#include <detail/byte_span.hpp>
#include <fstlog/detail/ut_cast.hpp>
#include <formatter/impl/detail/logfield.hpp>
#include <formatter/impl/detail/format_setting_txt.hpp>
#include <formatter/impl/detail/format_str_helper.hpp>
#include <detail/utf8_helper.hpp>

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
			buff_span_const form_spec) noexcept 
		{
			FSTLOG_ASSERT(field <= logfield_last);
			field_formattings_[ut_cast(field)] = get_format(form_spec);
        }

        format_type get_format(logfield field) const noexcept {
			FSTLOG_ASSERT(field <= logfield_last);
			return field_formattings_[ut_cast(field)];
        }

        static constexpr format_type get_default_format() noexcept {
            return format_type{};
        }

        static constexpr format_type get_format(
			buff_span_const form_spec) noexcept 
		{
            format_type out;
            if (form_spec.empty()) return out;

			const auto begin_pos{ form_spec.data() };
            auto pos = begin_pos;
            const auto end = pos + form_spec.size_bytes();

            const auto align_end = skip_align(pos, end);
            const std::size_t align_size = static_cast<std::size_t>(align_end - pos);

            pos = align_end;
            
            if (pos < end) {
                unsigned char sign_or_alt{ *pos };
                if (sign_or_alt == ' ' || sign_or_alt == '-' || sign_or_alt == '+') {
                    out.sign = sign_or_alt;
                    pos++;
                    if (pos < end) {
                        sign_or_alt = *pos;
                    }
                }
                if (sign_or_alt == '#') {
                    out.alternate = true;
                    pos++;
                }
            }

            //skip ["0"]
            if (*pos == '0') pos++;

            out.width = get_width(pos, end);
            if (out.width != 0) {
                
				if (align_size == 1) {
                    out.align = *begin_pos;
                }
                else if (align_size >= 2) {
					const auto utf_byte_num = valid_utf8(begin_pos, begin_pos + align_size - 1);
                    if (utf_byte_num != 0)
						memcpy(out.fill_char, begin_pos, utf_byte_num);
                    out.align = *(begin_pos + align_size - 1);
                }
                //if align_size == 0 use default align and fill char
            }

            out.precision = get_precision(pos, end);
            
            if (pos < end && *(end - 1) != 'L') {
                out.type = *(end - 1);
            }

            return out;
        }

		static constexpr format_type get_format_align_width(
			format_type format) noexcept
		{
			format_type out{};
			out.align = format.align;
			memcpy(out.fill_char, format.fill_char, sizeof(out.fill_char));
			out.width = format.width;
			return out;
		}

    private:
        std::array<format_type, ut_cast(logfield_last) + 1> field_formattings_;
    };
}
