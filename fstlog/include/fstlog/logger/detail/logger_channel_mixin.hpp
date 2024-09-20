//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/types.hpp>
#include <fstlog/detail/constants.hpp>

namespace fstlog {
	template<class L>
	class logger_channel_mixin : public L {
	public:
		channel_type channel() const noexcept {
			return channel_;
		}
		//not thread safe!!!
		void set_channel(channel_type channel) noexcept {
            channel_ = channel;
        }

        channel_type channel_{constants::default_log_channel};
	};
}
