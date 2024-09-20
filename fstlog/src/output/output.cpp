//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/output/output.hpp>

#include <cassert>

#include <output/output_interface.hpp>

namespace fstlog {
	output::output() noexcept = default;
	// Only useable by the factory methods. (pimpl acts like a token)
	output::output(output_interface* pimpl) noexcept 
		: pimpl_{ pimpl } 
	{
		if (pimpl != nullptr)
			pimpl->add_reference();
	}
	output::output(const output& other) noexcept
		: output(other.pimpl_) {}
	output& output::operator=(const output& other) noexcept {
		if (pimpl_ != other.pimpl_) {
			if (pimpl_ != nullptr) pimpl_->release_referred();
			pimpl_ = other.pimpl_;
			if (pimpl_ != nullptr) pimpl_->add_reference();
		}
		return *this;
	}
	output::output(output&& other) noexcept 
		: pimpl_{ other.pimpl_ } 
	{
		other.pimpl_ = nullptr;
	}
	output& output::operator=(output&& other) noexcept {
		assert(this != &other);
		if (pimpl_ != nullptr) pimpl_->release_referred();
		pimpl_ = other.pimpl_;
		other.pimpl_ = nullptr;
		return *this;
	}
	output::~output() noexcept {
		if (pimpl_ != nullptr)
			pimpl_->release_referred();
	}
	bool output::good() const noexcept {
		return pimpl_ != nullptr;
	}
	output_interface* output::pimpl() const noexcept {
		return pimpl_;
	}
}
