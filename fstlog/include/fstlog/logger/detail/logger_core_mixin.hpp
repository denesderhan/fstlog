//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/core.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
	template<class L>
	class logger_core_mixin : public L {
	public:
		void notify_core() noexcept	{
			FSTLOG_ASSERT(is_core_set() && "Core was not set!");
			core_.notify_data_ready();
		}

		core const& get_core() noexcept {
			return core_;
		}

		void set_core(core core) noexcept {
			core_ = std::move(core);
		}

		bool is_core_set() const noexcept {
			return (core_.good());
		}

		log_buffer get_buffer(std::uint32_t buffer_size) noexcept {
			return core_.get_buffer(buffer_size);
		}
		
		core core_{ nullptr };
	};
}
