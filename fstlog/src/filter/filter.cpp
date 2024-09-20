//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/filter/filter.hpp>

#include <cassert>

#include <fstlog/detail/fstlog_assert.hpp>
#include <detail/make_allocated.hpp>
#include <filter/filter_impl.hpp>

namespace fstlog {
	const char* filter::init(allocator_type const& allocator) noexcept {
		FSTLOG_ASSERT(pimpl_ == nullptr);
		pimpl_ = make_allocated<filter_impl>(allocator);
		if (pimpl_ == nullptr) return "Allocation failed (filter)!";
		return nullptr;
	}
	const char* filter::init(
		level level,
		channel_type channel,
		allocator_type const& allocator)  noexcept 
	{
		auto error = init(allocator);
		if (error == nullptr) {
			if (level == level::All)
				add_level(level::Fatal, level::Trace);
			else if (level != level::None)
				add_level(level::Fatal, level);
			add_channel(channel);
		}
		return error;
	}
	const char* filter::init(
		level level,
		channel_type first_channel,
		channel_type last_channel,
		allocator_type const& allocator) noexcept
	{
		auto error = init(allocator);
		if (error == nullptr) {
			if (level == level::All)
				add_level(level::Fatal, level::Trace);
			else if (level != level::None)
				add_level(level::Fatal, level);
			add_channel(first_channel, last_channel);
		}
		return error;
	}
	const char* filter::init(const filter& other) noexcept {
		if (pimpl_ != nullptr) return "Already initialized (filter)!";
		if (other.pimpl_ == nullptr) return nullptr;
		pimpl_ = make_allocated<filter_impl>(
			other.pimpl_->get_allocator(),
			*other.pimpl_);
		if (pimpl_ == nullptr) return "Allocation failed (filter)!";
		return nullptr;
	}
	const char* filter::init(const filter& other, allocator_type const& allocator) noexcept {
		if (pimpl_ != nullptr) return "Already initialized (filter)!";
		if (other.pimpl_ == nullptr) return nullptr;
		pimpl_ = make_allocated<filter_impl>(
			allocator, 
			*other.pimpl_);
		if (pimpl_ == nullptr) return "Allocation failed (filter)!";
		return nullptr;
	}
	filter& filter::operator=(const filter& other) noexcept {
		if (this != &other && pimpl_ != nullptr) {
			if (other.pimpl_ != nullptr) {
				pimpl_->message_filter_ = other.pimpl_->message_filter_;
			}
			else {
				const allocator_type allocator{ pimpl_->get_allocator() };
				pimpl_->~filter_impl();
				nothrow_deallocate(pimpl_, allocator);
				pimpl_ = nullptr;
			}
		}
		return *this;
	}
	filter::filter(filter&& other) noexcept 
		: pimpl_{ other.pimpl_ } 
	{
		other.pimpl_ = nullptr;
	}
	filter& filter::operator=(filter&& other) noexcept {
		assert(this != &other);
		if (pimpl_ != nullptr) {
			const allocator_type allocator{ pimpl_->get_allocator() };
			pimpl_->~filter_impl();
			nothrow_deallocate(pimpl_, allocator);
		}
		pimpl_ = other.pimpl_;
		other.pimpl_ = nullptr;
		return *this;
	}
	filter::~filter() noexcept {
		if (pimpl_ != nullptr) {
			const allocator_type allocator{ pimpl_->get_allocator() };
			pimpl_->~filter_impl();
			nothrow_deallocate(pimpl_, allocator);
			pimpl_ = nullptr;
		}
	}
	bool filter::good() const noexcept {
		return pimpl_ != nullptr;
	}

	void filter::add_level(level level) noexcept {
		if (good())
			pimpl_->message_filter_.add_level(level);
	}
	void filter::add_level(level first, level last) noexcept {
		if (good())
			pimpl_->message_filter_.add_level(first, last);
	}
	void filter::add_channel(channel_type channel) noexcept {
		if (good())
			pimpl_->message_filter_.add_channel(channel);
	}
	void filter::add_channel(channel_type first, channel_type last) noexcept {
		if (good())
			pimpl_->message_filter_.add_channel(first, last);
	}
	bool filter::filter_msg(level level, channel_type channel) const noexcept {
		if (good())
			return pimpl_->filter_msg(level, channel);
		else return false;
	}

	//this is needed, to be able to nothrow construct an empty filter
	filter::filter(filter_impl* pimpl) noexcept
		: pimpl_{ pimpl } {}
	filter_impl* filter::pimpl() const noexcept {
		return pimpl_;
	}
}
