//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#ifndef FSTLOG_MIN_BUFFERSIZE
#define FSTLOG_MIN_BUFFERSIZE 1024
#endif
#ifndef FSTLOG_MAX_BUFFERSIZE
#define FSTLOG_MAX_BUFFERSIZE 128*1024
#endif
#ifndef FSTLOG_DEFAULT_BUFFERSIZE
#define FSTLOG_DEFAULT_BUFFERSIZE 16*1024
#endif
#ifndef FSTLOG_MAX_MESSAGESIZE
#define FSTLOG_MAX_MESSAGESIZE 32*1024
#endif

#include <cstdint>

#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/padded_size.hpp>

namespace fstlog {
	namespace config {
		inline constexpr std::uint32_t min_ringbuffer_size{ FSTLOG_MIN_BUFFERSIZE };
		inline constexpr std::uint32_t max_ringbuffer_size{ FSTLOG_MAX_BUFFERSIZE };
		inline constexpr std::uint32_t default_ringbuffer_size{ FSTLOG_DEFAULT_BUFFERSIZE };
		inline constexpr std::uint32_t max_internal_log_msg_size{ FSTLOG_MAX_MESSAGESIZE };
		inline constexpr std::uint32_t minof_max_internal_log_msg_size{
			padded_size<constants::internal_msg_data_alignment>(std::uint32_t{128}) };
		
		inline constexpr auto buffer_alignment = 
			constants::cache_ls_nosharing > constants::internal_msg_alignment ? 
			constants::cache_ls_nosharing : constants::internal_msg_alignment;
		static_assert(is_pow2(constants::cache_ls_nosharing),
			"fstlog, cache_line_size must be power of two!");
		static_assert(is_pow2(constants::internal_msg_alignment),
			"fstlog, internal_msg_alignment must be power of two!");
		static_assert(is_pow2(buffer_alignment),
			"fstlog, log_buffer alignment must be power of two!");
		static_assert(is_pow2(max_ringbuffer_size),
			"fstlog, max_ringbuffer_size must be power of two!");
		static_assert(is_pow2(min_ringbuffer_size),
			"fstlog, min_ringbuffer_size must be power of two!");
		static_assert(is_pow2(default_ringbuffer_size),
			"fstlog, default_ringbuffer_size must be power of two!");
		static_assert(default_ringbuffer_size <= max_ringbuffer_size
			&& default_ringbuffer_size >= min_ringbuffer_size,
			"fstlog, default_ringbuffer_size out of range!");
		static_assert(min_ringbuffer_size <= max_ringbuffer_size,
			"fstlog, min_ringbuffer_size invalid value!");
		static_assert(min_ringbuffer_size >= minof_max_internal_log_msg_size,
			"fstlog, min_ringbuffer_size invalid value!");

		static_assert(max_internal_log_msg_size <= max_ringbuffer_size,
			"fstlog, max_internal_log_msg_size invalid value!");
		static_assert(max_internal_log_msg_size >= minof_max_internal_log_msg_size,
			"fstlog, max_internal_log_msg_size invalid value!");

		static_assert(max_internal_log_msg_size <=
			(std::numeric_limits<msg_counter>::max)(),
			"fstlog, max_internal_log_msg_size invalid value!");
	}
}
