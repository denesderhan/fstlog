//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/core.hpp>

#include <cassert>
#include <limits>

#include <config_core.hpp>
#include <core_impl.hpp>
#include <detail/make_allocated.hpp>

namespace fstlog {
	const char* core::init(allocator_type const& allocator) noexcept {
		return init("Unnamed", allocator);
	}
	const char* core::init(std::string_view name, allocator_type const& allocator) noexcept {
		if (pimpl_ != nullptr) return "Already initialized (core)!";
		pimpl_ = make_allocated<core_impl>(allocator, name);
		if (pimpl_ == nullptr) return "Allocation failed (core obj.)!";
		pimpl_->add_reference();
		const auto error = pimpl_->init();
		if (error != nullptr) return error;
		return nullptr;
	}

	//this is needed, to be able to construct an empty core (core{nullptr} in logger_core_mixin)
	core::core(core_impl* pimpl) noexcept 
		: pimpl_{ pimpl } 
	{
		if (pimpl != nullptr)
			pimpl->add_reference();
	}

	core::~core() noexcept {
		if (pimpl_ != nullptr) {
			if (pimpl_->remove_reference()) {
				const allocator_type allocator{ pimpl_->get_allocator() };
				pimpl_->~core_impl();
				nothrow_deallocate(pimpl_, allocator);
				pimpl_ = nullptr;
			}
		}
	}

	core::core(const core& other) noexcept 
		:core(other.pimpl_) {}
	core& core::operator=(const core& other) noexcept {
		if (pimpl_ != other.pimpl_) {
			//temp is created to call destructor (deallocate) if last ref.
			if (pimpl_ != nullptr) {
				core temp(pimpl_);
				pimpl_->remove_reference();
			}
			pimpl_ = other.pimpl_;
			if (pimpl_ != nullptr) pimpl_->add_reference();
		}
		return *this;
	}
	core::core(core&& other) noexcept 
		:pimpl_{ other.pimpl_ } 
	{
		other.pimpl_ = nullptr;
	}
	core& core::operator=(core&& other) noexcept {
		assert(this != &other);
		//temp is created to call destructor (deallocate) if last ref.
		if (pimpl_ != nullptr) {
			core temp(pimpl_);
			pimpl_->remove_reference();
		}
		pimpl_ = other.pimpl_;
		other.pimpl_ = nullptr;
		return *this;
	}

	bool core::start() noexcept {
		if (good()) { 
			pimpl_->start();
		}
		return running();
	}
	bool core::stop() noexcept {
		if (good()) {
			pimpl_->stop();
		}
		return !running();
	}
	bool core::restart() noexcept {
		bool success{ false };
		if (good()) {
			success = stop();
			if (success) {
				success = start();
			}
		}
		return success;
	}
	bool core::running() const noexcept {
		if (good()) {
			return pimpl_->running();
		}
		else {
			return false;
		}
	}
	std::chrono::milliseconds core::poll_interval(std::chrono::milliseconds poll_interval) noexcept {
		if (good()) {
			return pimpl_->poll_interval(poll_interval);
		}
		else {
			return std::chrono::milliseconds{ 0 };
		}
	}
	std::chrono::milliseconds core::poll_interval() const noexcept {
		if (good()) {
			return pimpl_->poll_interval();
		}
		else {
			return std::chrono::milliseconds{ 0 };
		}
	}
	bool core::add_sink(sink sink) noexcept {
		if (!good() || !sink.good()) {
			return false;
		}
		else {
			return pimpl_->add_sink(std::move(sink));
		}
	}
	bool core::release_sink(sink& sink) noexcept {
		if (good() && sink.good()) {
			return pimpl_->release_sink(sink.pimpl());
		}
		else {
			return false;
		}
	}
	void core::flush() const noexcept {
		if (good()) {
			pimpl_->flush();
		}
	}

    std::string_view core::name() const noexcept {
		if (good()) {
			return pimpl_->name();
		}
		else {
			return std::string_view{ "" };
		}
	}
	std::string_view core::version() const noexcept {
		if (good()) {
			return pimpl_->version();
		}
		else {
			return std::string_view{ "0.0.0" };
		}
	}
	int core::version_major() const noexcept {
		if (good()) {
			return pimpl_->version_major();
		}
		else {
			return -1;
		}
	}
	int core::version_minor() const noexcept {
		if (good()) {
			return pimpl_->version_minor();
		}
		else {
			return -1;
		}
	}
	int core::version_patch() const noexcept {
		if (good()) {
			return pimpl_->version_patch();
		}
		else {
			return -1;
		}
	}
	bool core::good() const noexcept {
		return pimpl_ != nullptr;
	}

	void core::notify_data_ready() const noexcept {
		if (good()) {
			pimpl_->notify_data_ready();
		}
	}

	log_buffer core::get_buffer(std::uint32_t buffer_size) noexcept {
		if (good()) {
			return pimpl_->get_buffer(buffer_size);
		}
		else {
			return log_buffer{};
		}
	}

	std::uintmax_t core::id() const noexcept {
		if (good()) {
			return pimpl_->id();
		}
		else {
			return (std::numeric_limits<std::uintmax_t>::max)();
		}
	}

	core_impl* core::pimpl() const noexcept {
		return pimpl_;
	}
}
