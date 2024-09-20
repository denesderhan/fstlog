//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/sink/sink.hpp>

#include <cassert>

#include <sink/sink_interface.hpp>

namespace fstlog {
	sink::sink()  noexcept = default;
	// Only useable by the factory methods. (pimpl acts like a token)
	sink::sink(sink_interface* pimpl) noexcept 
		:pimpl_{ pimpl } 
	{
		if (pimpl != nullptr)
			pimpl->add_reference();
	}
	sink::sink(const sink& other) noexcept
		: sink(other.pimpl_) {}
	sink& sink::operator=(const sink& other) noexcept {
		if (pimpl_ != other.pimpl_) {
			if (pimpl_ != nullptr) pimpl_->release_referred();
			pimpl_ = other.pimpl_;
			if (pimpl_ != nullptr) pimpl_->add_reference();
		}
		return *this;
	}
	sink::sink(sink&& other) noexcept 
		:pimpl_{ other.pimpl_ }
	{
		other.pimpl_ = nullptr;
	}
	sink& sink::operator=(sink&& other) noexcept {
		assert(this != &other);
		if (pimpl_ != nullptr) pimpl_->release_referred();
		pimpl_ = other.pimpl_;
		other.pimpl_ = nullptr;
		return *this;
	}
	sink::~sink()  noexcept {
		if (pimpl_ != nullptr)
			pimpl_->release_referred();
	}
	bool sink::good() const noexcept {
		return pimpl_ != nullptr;
	}
	sink_interface* sink::pimpl() const noexcept {
		return pimpl_;
	}
}
