//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <mutex>

#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
    template<class L>
    class mutex_external_mixin : public L
    {
    public:
        using allocator_type = typename L::allocator_type;
        
        mutex_external_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(mutex_external_mixin(allocator_type{})))
			: mutex_external_mixin(allocator_type{}) {}
		explicit mutex_external_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
            : L(allocator) {}

        //No copy constructor! copy would use the same mutex!
        mutex_external_mixin(const mutex_external_mixin& other) = delete;
        mutex_external_mixin(mutex_external_mixin&& other) = delete;
        mutex_external_mixin& operator=(const mutex_external_mixin& rhs) = delete;
        mutex_external_mixin& operator=(mutex_external_mixin&& rhs) = delete;
       
        ~mutex_external_mixin() = default;

		const char* set_mutex(std::shared_ptr<std::mutex> mutex) noexcept {
			sync_mutex_ = std::move(mutex);
			if (sync_mutex_ == nullptr) return "Mutex was nullptr!";
			return nullptr;
        }

        std::mutex& get_mutex() noexcept {
            FSTLOG_ASSERT(sync_mutex_ != nullptr);
			return *sync_mutex_.get();
        }

    private:
        std::shared_ptr<std::mutex> sync_mutex_;
    };
}
