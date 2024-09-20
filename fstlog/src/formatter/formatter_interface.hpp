//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <string_view>

#include <detail/byte_span.hpp>
#include <fstlog/detail/fstlog_allocator.hpp>
#include <fstlog/formatter/formatter.hpp>

namespace fstlog {
	class formatter_interface
	{
	protected:
		formatter_interface() noexcept = default;
		formatter_interface(const formatter_interface&) noexcept = default;
		formatter_interface& operator=(const formatter_interface&) = delete;
		formatter_interface(formatter_interface&&) = delete;
		formatter_interface& operator=(formatter_interface&&) = delete;
		virtual ~formatter_interface() = default;
	
	public:
		virtual buff_span format_message(
			buff_span_const in,
			buff_span out) noexcept = 0;
		virtual const char* clone(formatter& out) const noexcept = 0;
		virtual const char* clone(
			formatter& out,
			fstlog_allocator const& allocator) const noexcept = 0;
		virtual bool use() noexcept = 0;
		virtual void release() noexcept = 0;
	
	private:
		virtual void add_reference() noexcept = 0;
		virtual void release_referred() noexcept = 0;
		friend class formatter;
	};
}
