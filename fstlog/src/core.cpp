//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/core.hpp>

#include <cassert>
#include <limits>

#include <config_core.hpp>
#include <core_impl.hpp>
#include <detail/make_allocated.hpp>
#include <fstlog/version.hpp>

namespace fstlog {
    error_code core::init(allocator_type const& allocator) noexcept {
        return init("Unnamed", allocator);
    }
    error_code core::init(std::string_view name, allocator_type const& allocator) noexcept {
        if (pimpl_ != nullptr) return error_code::double_init;
        pimpl_ = make_allocated<core_impl>(allocator, name);
        if (pimpl_ == nullptr) return error_code::alloc_fail;
        pimpl_->add_reference();
        const auto error = pimpl_->init();
        return error;
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

    std::string_view core::version() noexcept {
        return FSTLOG_VERSION;
    }
    int core::version_major() noexcept {
        return FSTLOG_VERSION_MAJOR;
    }
    int core::version_minor() noexcept {
        return FSTLOG_VERSION_MINOR;
    }
    int core::version_patch() noexcept {
        return FSTLOG_VERSION_PATCH;
    }

    bool core::good() const noexcept {
        return pimpl_ != nullptr;
    }

    void core::detail_notify_data_ready() const noexcept {
        if (good()) {
            pimpl_->notify_data_ready();
        }
    }

    log_buffer core::detail_get_buffer(std::uint32_t buffer_size) noexcept {
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
            return 0;
        }
    }

    log_buffer& core::detail_tls_buffer() noexcept {
        assert(pimpl_ != nullptr);
        return pimpl_->tls_buffer();
    }

    core_impl* core::pimpl() const noexcept {
        return pimpl_;
    }
}
