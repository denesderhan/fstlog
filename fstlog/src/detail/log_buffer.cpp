//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/detail/log_buffer.hpp>

#include <cassert>

#include <detail/log_buffer_impl.hpp>
#include <detail/nothrow_allocate.hpp>

namespace fstlog {

	log_buffer::log_buffer() noexcept = default;
	log_buffer::log_buffer(log_buffer_impl* pimpl) noexcept 
		:pimpl_{ pimpl } 
	{
		if (pimpl_ != nullptr) {
			pimpl_->add_reference();
		}
	}

	log_buffer::~log_buffer() noexcept {
		if (pimpl_ != nullptr) {
			if (pimpl_->remove_reference()) {
				const log_buffer_impl::allocator_type allocator{ pimpl_->get_allocator() };
				pimpl_->~log_buffer_impl();
				nothrow_deallocate(pimpl_, allocator);
				pimpl_ = nullptr;
			}
		}
	}
	log_buffer::log_buffer(const log_buffer& other) noexcept 
		: log_buffer(other.pimpl_){}
	log_buffer& log_buffer::operator=(const log_buffer& other) noexcept {
		if (pimpl_ != other.pimpl_) {
			log_buffer temp;
			//temporary created to automatically call destructor (deallocating)
			temp.pimpl_ = pimpl_;
			pimpl_ = other.pimpl_;
			if (pimpl_ != nullptr) pimpl_->add_reference();
		}
		return *this;
	}
	log_buffer::log_buffer(log_buffer&& other) noexcept {
		pimpl_ = other.pimpl_;
		other.pimpl_ = nullptr;
	}
	log_buffer& log_buffer::operator=(log_buffer&& other) noexcept {
		assert(this != &other);
		log_buffer temp;
		//temporary created to automatically call destructor (deallocating)
		temp.pimpl_ = pimpl_;
		pimpl_ = other.pimpl_;
		other.pimpl_ = nullptr;
		return *this;
	}
	bool log_buffer::good() const noexcept {
		return pimpl_ != nullptr && pimpl_->good();
	}

	std::uint32_t log_buffer::size() const noexcept {
		return pimpl_->size();
	}
	std::uint32_t log_buffer::max_message_size() const noexcept {
		return pimpl_->max_message_size();
	}
	std::uint32_t log_buffer::writeable_size() const noexcept {
		return pimpl_->writeable_size();
	}
	void log_buffer::update_producer_state(std::uint32_t needed_size) noexcept {
		pimpl_->update_producer_state(needed_size);
	}
	void log_buffer::wait_for_buffer_flush(core const& core) noexcept {
		pimpl_->wait_for_buffer_flush(core);
	}
	std::uint32_t log_buffer::write_pos() const noexcept {
		return pimpl_->write_pos();
	}
	void log_buffer::request_flush_if_needed(
		std::uint32_t prev_write_pos,
		core const& core) noexcept 
	{
		pimpl_->request_flush_if_needed(prev_write_pos, core);
	}
	unsigned char* log_buffer::write_ptr() const noexcept {
		return pimpl_->write_ptr();
	}
	void log_buffer::advance_write_pos(std::uint32_t adv_size) noexcept {
		pimpl_->advance_write_pos(adv_size);
	}
	log_buffer_impl* log_buffer::pimpl() const noexcept {
		return pimpl_;
	}
}
