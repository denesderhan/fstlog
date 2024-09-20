//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>
#include <fstlog/detail/api_def.hpp>

namespace fstlog {
	class core;
	class log_buffer_impl;
	class log_buffer {
	public:
		FSTLOG_API log_buffer() noexcept;
		FSTLOG_API ~log_buffer() noexcept;
		FSTLOG_API log_buffer(const log_buffer& other) noexcept;
		FSTLOG_API log_buffer& operator=(const log_buffer& other) noexcept;
		FSTLOG_API log_buffer(log_buffer&& other) noexcept;
		FSTLOG_API log_buffer& operator=(log_buffer&& other) noexcept;
		
		FSTLOG_API bool good() const noexcept;

		FSTLOG_API std::uint32_t size() const noexcept;
		FSTLOG_API std::uint32_t max_message_size() const noexcept;
		FSTLOG_API std::uint32_t writeable_size() const noexcept;
		FSTLOG_API void update_producer_state(std::uint32_t needed_size) noexcept;
		FSTLOG_API void wait_for_buffer_flush(core const& core) noexcept;
		FSTLOG_API std::uint32_t write_pos() const noexcept;
		FSTLOG_API void request_flush_if_needed(
			std::uint32_t prev_write_pos,
			core const& core) noexcept;
		FSTLOG_API unsigned char* write_ptr() const noexcept;
		FSTLOG_API void advance_write_pos(std::uint32_t adv_size) noexcept;

		explicit log_buffer(log_buffer_impl* pimpl) noexcept;
		log_buffer_impl* pimpl() const noexcept;
	private:
		log_buffer_impl* pimpl_{ nullptr };
	};
}
