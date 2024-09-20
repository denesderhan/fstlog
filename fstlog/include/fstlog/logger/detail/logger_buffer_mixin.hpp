//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <limits>
#include <cstdint>

#include <fstlog/core.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/log_buffer.hpp>

namespace fstlog {
	template<class L>
	class logger_buffer_mixin : public L {
	public:

		void set_core(core core) noexcept(
			noexcept(L::set_core(fstlog::core{ nullptr })))
		{
			if (core.pimpl() != L::get_core().pimpl()) {
				log_buffer_ = log_buffer{};
				L::set_core(std::move(core));
			}
		}

		void new_buffer() noexcept(noexcept(buffer().size()))
		{
			std::uint32_t buff_size{ 0 };
			if (is_buffer_set()) {
				buff_size = buffer().size();
			}
			new_buffer(buff_size);
		}

		void new_buffer(std::uint32_t buffer_size) noexcept	{
			if (this->is_core_set()) {
				set_buffer(this->get_buffer(buffer_size));
			}
		}

		bool is_buffer_set() const noexcept {
			return log_buffer_.pimpl() != nullptr;
		}

		log_buffer& buffer() noexcept {
			FSTLOG_ASSERT(log_buffer_.pimpl() != nullptr && "Error, log_buffer_ was nullptr!");
			return { log_buffer_ };
		}
		void set_buffer(log_buffer buffer) noexcept {
			FSTLOG_ASSERT(log_buffer_.pimpl() == nullptr || log_buffer_.good());
			log_buffer_ = std::move(buffer);
		}

		log_buffer log_buffer_{};
	};
}
