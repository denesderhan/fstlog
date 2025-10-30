//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/filter/filter.hpp>

#include <cassert>

#include <detail/make_allocated.hpp>
#include <filter/filter_impl.hpp>

namespace fstlog {
    error_code filter::init(memory_resource* resource) noexcept {
        if (pimpl_ != nullptr) return error_code::double_init;
        pimpl_ = make_allocated<filter_impl>(resource);
        if (pimpl_ == nullptr) return error_code::alloc_fail;
        pimpl_->set_memory_resource(resource);
        return error_code::none;
    }
    error_code filter::init(
        level level,
        channel_type channel,
        memory_resource* resource)  noexcept 
    {
        auto error = init(resource);
        if (error == error_code::none) {
            if (level == level::All)
                add_level(level::Fatal, level::Trace);
            else if (level != level::None)
                add_level(level::Fatal, level);
            add_channel(channel);
        }
        return error;
    }
    error_code filter::init(
        level level,
        channel_type first_channel,
        channel_type last_channel,
        memory_resource* resource) noexcept
    {
        auto error = init(resource);
        if (error == error_code::none) {
            if (level == level::All)
                add_level(level::Fatal, level::Trace);
            else if (level != level::None)
                add_level(level::Fatal, level);
            add_channel(first_channel, last_channel);
        }
        return error;
    }
    error_code filter::init(const filter& other) noexcept {
        if (pimpl_ != nullptr) return error_code::double_init;
        if (other.pimpl_ == nullptr) return error_code::none;
        return init(other, other.pimpl_->get_memory_resource());
    }
    error_code filter::init(const filter& other, memory_resource* resource) noexcept {
        if (pimpl_ != nullptr) return error_code::double_init;
        if (other.pimpl_ == nullptr) return error_code::none;
        pimpl_ = make_allocated<filter_impl>(resource, *other.pimpl_);
        if (pimpl_ == nullptr) return error_code::alloc_fail;
        pimpl_->set_memory_resource(resource);
        return error_code::none;
    }
    filter& filter::operator=(const filter& other) noexcept {
        if (this != &other 
            && pimpl_ != nullptr
            && other.pimpl_ != nullptr)
        {
           pimpl_->message_filter_ = other.pimpl_->message_filter_;
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
            memory_resource* resource{ pimpl_->get_memory_resource() };
            pimpl_->~filter_impl();
            nothrow_deallocate(pimpl_, resource);
        }
        pimpl_ = other.pimpl_;
        other.pimpl_ = nullptr;
        return *this;
    }

    bool filter::operator==(const filter& other) const noexcept {
        if (pimpl_ == nullptr && other.pimpl_ == nullptr) return true;
        if (pimpl_ == nullptr || other.pimpl_ == nullptr) return false;
        return pimpl_->message_filter_ == other.pimpl_->message_filter_;
    }

    bool filter::operator!=(const filter& other) const noexcept {
        return !(*this == other);
    }

    filter::~filter() noexcept {
        if (pimpl_ != nullptr) {
            memory_resource* resource{ pimpl_->get_memory_resource() };
            pimpl_->~filter_impl();
            nothrow_deallocate(pimpl_, resource);
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
