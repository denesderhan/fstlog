//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>
#include <chrono>

#include <detail/error_code.hpp>

namespace fstlog {
	class sink_interface
	{
		typedef std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> steady_msec;
	
	protected:
		sink_interface() noexcept = default;
		sink_interface(const sink_interface&) = delete;
		sink_interface& operator=(const sink_interface&) = delete;
		sink_interface(sink_interface&&) = delete;
		sink_interface& operator=(sink_interface&&) = delete;
		virtual ~sink_interface() = default;
	
	public:	
		virtual error_code sink_msg_block(const unsigned char* dat_ptr, std::uint32_t dat_size) noexcept = 0;
		virtual bool needs_immediate_flush() const noexcept = 0;
		virtual steady_msec next_flush_time() const noexcept = 0;
		virtual void flush(steady_msec current_time) noexcept = 0;
		virtual bool use() noexcept = 0;
		virtual void release() noexcept = 0;
	
	private:
		virtual void add_reference() noexcept = 0;
		virtual void release_referred() noexcept = 0;
		friend class sink;
	};
}
