//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/formatter/formatter.hpp>

#include <cassert>

#include <formatter/formatter_interface.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
	formatter::formatter() noexcept = default;
	formatter::formatter(formatter_interface* pimpl) noexcept 
		: pimpl_{ pimpl } 
	{
		if (pimpl != nullptr) 
			pimpl->add_reference();
	}
	formatter::~formatter() noexcept {
		if (pimpl_ != nullptr)
			pimpl_->release_referred();
	}
	formatter::formatter(const formatter& other) noexcept 
		: formatter(other.pimpl_) {}
	formatter& formatter::operator=(const formatter& other) noexcept {
		if (pimpl_ != other.pimpl_) {
			if (pimpl_ != nullptr) pimpl_->release_referred();
			pimpl_ = other.pimpl_;
			if (pimpl_ != nullptr) pimpl_->add_reference();
		}
		return *this;
	}
	formatter::formatter(formatter&& other) noexcept
		:pimpl_{ other.pimpl_}
	{
		other.pimpl_ = nullptr;
	}
	formatter& formatter::operator=(formatter&& other) noexcept {
		assert(this != &other);
		if (pimpl_ != nullptr) pimpl_->release_referred();
		pimpl_ = other.pimpl_;
		other.pimpl_ = nullptr;
		return *this;
	}

	const char* formatter::clone(formatter& out) const noexcept {
		if (pimpl_ == nullptr) {
			out = formatter{};
			return nullptr;
		}
		else {
			return pimpl_->clone(out);
		}
	}
	const char* formatter::clone(
		formatter& out,
		fstlog_allocator const& allocator) const noexcept
	{
		if (pimpl_ == nullptr) {
			out = formatter{};
			return nullptr;
		}
		else {
			return pimpl_->clone(out, allocator);
		}
	}
	bool formatter::good() const noexcept {
		return pimpl_ != nullptr;
	}

	formatter_interface* formatter::pimpl() const noexcept {
		return pimpl_;
	}
}
