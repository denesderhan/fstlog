//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <mutex>
#include <type_traits>

#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
    template<class L>
    class mutex_external_mixin : public L
    {
    public:
        mutex_external_mixin() noexcept = default;

        //No copy constructor! copy would use the same mutex!
        mutex_external_mixin(const mutex_external_mixin& other) = delete;
        mutex_external_mixin(mutex_external_mixin&& other) = delete;
        mutex_external_mixin& operator=(const mutex_external_mixin& rhs) = delete;
        mutex_external_mixin& operator=(mutex_external_mixin&& rhs) = delete;
       
        ~mutex_external_mixin() = default;

        error_code set_mutex(std::shared_ptr<std::mutex> mutex) noexcept {
            sync_mutex_ = std::move(mutex);
            if (sync_mutex_ == nullptr) return error_code::obj_null;
            return error_code::none;
        }

        std::mutex& get_mutex() noexcept {
            FSTLOG_ASSERT(sync_mutex_ != nullptr);
            return *sync_mutex_.get();
        }

    private:
        std::shared_ptr<std::mutex> sync_mutex_;
    };
}
