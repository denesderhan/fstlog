//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstddef>
#include <cstring>
#pragma intrinsic(memcpy)

#include <detail/byte_span.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <fstlog/detail/small_string.hpp>
#include <fstlog/detail/ut_cast.hpp>
#include <formatter/impl/detail/format_str_helper.hpp>
#include <formatter/impl/detail/logfield.hpp>

namespace fstlog {
    template<typename L>
    class logfield_formspec_fmt_mixin : public L
    {
    public:
        using allocator_type = typename L::allocator_type;
        typedef small_string<24> format_type;

        logfield_formspec_fmt_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(logfield_formspec_fmt_mixin(allocator_type{})))
			: logfield_formspec_fmt_mixin(allocator_type{}) {}
        explicit logfield_formspec_fmt_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
			: L(allocator)
		{
            field_formattings_.fill(get_default_format()); //noexcept
        }

        logfield_formspec_fmt_mixin(const logfield_formspec_fmt_mixin& other) noexcept(
			noexcept(logfield_formspec_fmt_mixin::get_allocator())
			&& noexcept(logfield_formspec_fmt_mixin(logfield_formspec_fmt_mixin{}, allocator_type{})))
            : logfield_formspec_fmt_mixin(other, other.get_allocator()) {}
        logfield_formspec_fmt_mixin(const logfield_formspec_fmt_mixin& other, allocator_type const& allocator) noexcept(
			noexcept(L(logfield_formspec_fmt_mixin{}, allocator_type{})))
            : L(other, allocator),
            field_formattings_{ other.field_formattings_ } {} //noexcept
        
        logfield_formspec_fmt_mixin(logfield_formspec_fmt_mixin&& other) = delete;
        logfield_formspec_fmt_mixin& operator=(const logfield_formspec_fmt_mixin& rhs) = delete;
        logfield_formspec_fmt_mixin& operator=(logfield_formspec_fmt_mixin&& rhs) = delete;
        
        ~logfield_formspec_fmt_mixin() = default;
        
    public:
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
            return format_type{ std::string_view{"{}"} };
        }

        static constexpr format_type get_format(
			buff_span_const form_spec) noexcept 
		{
            const std::size_t form_spec_size = form_spec.size_bytes();
			if (form_spec.empty() || form_spec_size > 20)
				return get_default_format();
			std::array<char, 24> temp_str{"{:}}}}}}}}}}}}}}}}}}}}}"};
            memcpy(&temp_str[2], form_spec.data(), form_spec_size);
			return format_type{ std::string_view{ &temp_str[0], form_spec_size + 3 } };
        }

		static constexpr format_type get_format_align_width(
			format_type format) noexcept 
		{
			if (format.size() <= 3) return get_default_format();
			std::array<unsigned char, 24> out{ "{:}}}}}}}}}}}}}}}}}}}}}" };
			const auto form_beg{ safe_reinterpret_cast<const unsigned char*>(format.data()) };
			const auto end{ form_beg + format.size() - 1 };
			auto align_beg = form_beg + 2;
			auto align_end = skip_align(align_beg, end);
			auto out_end_pos{ out.data() + 2 };
			while (align_beg < align_end) {
				*out_end_pos++ = *align_beg++;
			}
			auto width_beg = skip_sign_alt_0(align_end, end);
			auto width_end = skip_numbers(width_beg, end);
			while (width_beg < width_end) {
				*out_end_pos++ = *width_beg++;
			}
			const std::size_t out_size = ++out_end_pos - out.data();
			return format_type{ safe_reinterpret_cast<const char*>(out.data()),  out_size };
		}

    private:
		std::array<format_type, ut_cast(logfield_last) + 1> field_formattings_;
    };
}
