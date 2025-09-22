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
    error_code core::init(memory_resource* resource) noexcept {
        return init("Unnamed", resource);
    }
    error_code core::init(std::string_view name, memory_resource* resource) noexcept {
        if (pimpl_ != nullptr) return error_code::double_init;
        pimpl_ = make_allocated<core_impl>(resource, name);
        if (pimpl_ == nullptr) return error_code::alloc_fail;
        const auto error = pimpl_->init();
        if (error != error_code::none) {
            pimpl_->~core_impl();
            nothrow_deallocate(pimpl_, resource);
            pimpl_ = nullptr;
        }
        else {
            pimpl_->add_reference();
        }
        return error;
    }

    //this is needed, to be able to construct an empty core (core{nullptr} in logger_core_mixin)
    core::core(std::nullptr_t) noexcept {}

    core::core(core_impl* pimpl) noexcept
        : pimpl_{ pimpl } 
    {
        if (pimpl != nullptr)
            pimpl->add_reference();
    }

    core::~core() noexcept {
        if (pimpl_ != nullptr) {
            if (pimpl_->remove_reference()) {
                memory_resource* resource{ pimpl_->get_memory_resource() };
                pimpl_->~core_impl();
                nothrow_deallocate(pimpl_, resource);
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
    bool core::operator==(const core& other) const noexcept {
        return pimpl_ == other.pimpl_;
    }
    bool core::operator!=(const core& other) const noexcept {
        return !(*this == other);
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

    bool core::good() const noexcept {
        return pimpl_ != nullptr;
    }

    void core::detail_notify_data_ready() const noexcept {
        FSTLOG_ASSERT(pimpl_ != nullptr);
        pimpl_->notify_data_ready();
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

    std::size_t core::limit() noexcept {
        return config::core_instance_limit;
    }

    log_buffer& core::detail_tls_buffer() noexcept {
        assert(pimpl_ != nullptr);
        return pimpl_->tls_buffer();
    }

    core_impl* core::pimpl() const noexcept {
        return pimpl_;
    }
}
