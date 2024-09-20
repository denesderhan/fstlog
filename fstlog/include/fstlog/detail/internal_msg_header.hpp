//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>

#include <fstlog/detail/types.hpp>
#include <fstlog/detail/level.hpp>
#include <fstlog/detail/log_policy.hpp>
#ifdef FSTLOG_DEBUG
#include <fstlog/detail/padded_size.hpp>
#include <fstlog/detail/constants.hpp>
#endif

namespace fstlog {
	enum class log_msg_type : unsigned char {
		Invalid = 0,
		Internal = 1,
		Binary = 2
	};

	struct internal_msg_header
	{
		log_msg_type msg_type{ log_msg_type::Invalid };
		level severity;
		channel_type channel;
		log_call_flag flags;
		msg_counter msg_size;
		msg_counter argnum;
		stamp_type timestamp;
		log_policy policy;
		unsigned char version[3]{1, 0, 0};
		
		static constexpr std::size_t unpadded_data_size =
			sizeof(msg_type) +
			sizeof(severity) +
			sizeof(channel) +
			sizeof(flags) +
			sizeof(msg_size) +
			sizeof(argnum) +
			sizeof(timestamp) +
			sizeof(policy) +
			sizeof(version);
		static constexpr std::size_t padded_data_size =
			unpadded_data_size;
#ifdef FSTLOG_DEBUG
		static_assert(padded_data_size == padded_size<constants::internal_msg_data_alignment>(unpadded_data_size));
#endif
	};
#ifdef FSTLOG_DEBUG
	static_assert(
		offsetof(internal_msg_header, version) + sizeof(internal_msg_header::version) 
		== internal_msg_header::unpadded_data_size);
#endif
}
