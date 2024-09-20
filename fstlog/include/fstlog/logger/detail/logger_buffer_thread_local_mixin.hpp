//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <limits>
#include <cstdint>

#include <fstlog/core.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/log_buffer.hpp>

namespace fstlog {
	template<class L>
	class logger_buffer_thread_local_mixin : public L {
	public:
		logger_buffer_thread_local_mixin() noexcept = default;
		~logger_buffer_thread_local_mixin() noexcept = default;

		logger_buffer_thread_local_mixin(const logger_buffer_thread_local_mixin& other) noexcept = default;
		logger_buffer_thread_local_mixin& operator=(const logger_buffer_thread_local_mixin& other) noexcept = default;

		logger_buffer_thread_local_mixin(logger_buffer_thread_local_mixin&& other) noexcept
			: L(std::move(other)),
			core_ind_{ other.core_ind_ }
		{
			other.core_ind_ = -1;
		}
				
		logger_buffer_thread_local_mixin& operator=(logger_buffer_thread_local_mixin&& other) noexcept {
			L::operator=(std::move(other));
			core_ind_ = other.core_ind_;
			other.core_ind_ = -1;
			return *this;
		}

		static void new_buffer(core& core) noexcept(noexcept(log_buffer{}.size())) {
			const auto core_id{ core.id() };
			if (core_id >= 0 && core_id < 32) {
				std::uint32_t buffer_size{ 0 };
				log_buffer& buffer{ log_buffers_[core_id] };
				if (buffer.pimpl() != nullptr) {
					buffer_size = buffer.size();
				}
				buffer = core.get_buffer(buffer_size);
			}
		}

		static void new_buffer(core& core, std::uint32_t buffer_size) noexcept {
			const auto core_id{ core.id() };
			if (core_id >= 0 && core_id < 32) {
				log_buffers_[core_id] = core.get_buffer(buffer_size);
			}
		}

		static void release_buffer(const core& core) noexcept {
			const auto core_id{ core.id() };
			if (core_id >= 0 && core_id < 32) {
				log_buffers_[core_id] = log_buffer{};
			}
		}

		bool is_buffer_set() const noexcept {
			FSTLOG_ASSERT(core_ind_ >=0 && core_ind_ < 32);
			return log_buffers_[core_ind_].pimpl() != nullptr;
		}

		log_buffer& buffer() noexcept {
			FSTLOG_ASSERT(core_ind_ >= 0 && core_ind_ < 32);
			FSTLOG_ASSERT(log_buffers_[core_ind_].pimpl() != nullptr && "Error, log_buffer.pimpl() was nullptr!");
			return { log_buffers_[core_ind_] };
		}
		void set_buffer(log_buffer buffer) noexcept {
			FSTLOG_ASSERT(core_ind_ >= 0 && core_ind_ < 32);
			FSTLOG_ASSERT(log_buffers_[core_ind_].pimpl() == nullptr || log_buffers_[core_ind_].good());
			log_buffers_[core_ind_] = std::move(buffer);
		}

		// not thread safe
		void set_core(core core) noexcept(
			noexcept(L::set_core(fstlog::core{ nullptr })))
		{
			if (core.pimpl() != L::get_core().pimpl()
				&& core.id() < 32)
			{
				core_ind_ = static_cast<int>(core.id());
				L::set_core(std::move(core));
			}
		}
		
		alignas(constants::cache_ls_nosharing) thread_local inline static log_buffer log_buffers_[32]{};
		int core_ind_{ -1 };
	};
}
