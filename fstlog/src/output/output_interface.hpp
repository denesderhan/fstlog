//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <detail/byte_span.hpp>

namespace fstlog {
	class output_interface 
	{
	protected:
		output_interface() noexcept = default;
		output_interface(const output_interface&) = delete;
		output_interface& operator=(const output_interface&) = delete;
		output_interface(output_interface&&) = delete;
		output_interface& operator=(output_interface&&) = delete;
		virtual ~output_interface() = default;
	
	public:
		virtual void write_message(buff_span_const msg) noexcept = 0;
		virtual void flush() noexcept = 0;
		virtual bool use() noexcept = 0;
		virtual void release() noexcept = 0;
	
	private:
		virtual void add_reference() noexcept = 0;
		virtual void release_referred() noexcept = 0;
		friend class output;
	};
}
