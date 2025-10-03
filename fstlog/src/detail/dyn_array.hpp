//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cassert>
#include <cstdint>
#include <utility>
#include <memory>
#include <new>

#include <fstlog/detail/memory_resource.hpp>
#include <detail/nothrow_allocate.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
    template <typename T>
    class dyn_array {
    public:
        using value_type = T;

        explicit dyn_array(memory_resource* resource) noexcept
            : memory_resource_{ resource } 
        {
            FSTLOG_ASSERT(resource != nullptr && "Memory resource must not be null!");
        }
        
        ~dyn_array() noexcept {
            clear();
            if (begin_ != nullptr) {
                nothrow_deallocate(begin_, memory_resource_, capacity());
            }
            begin_ = nullptr;
            end_ = nullptr;
            cap_end_ = nullptr;
        }

        dyn_array(dyn_array const&) = delete;
        dyn_array& operator=(dyn_array const&) = delete;
        dyn_array(dyn_array&&) = delete;
        dyn_array& operator=(dyn_array&&) = delete;
        
        // iterators are invalidated
        template <typename U>
        bool try_push_back(U&& element) noexcept {
            // if full make space
            if (full()) {
                if (!grow()) return false;
            }
            ::new (static_cast<void*>(end_)) T(std::forward<U>(element));
            ++end_;
            return true;
        }

        // removes the element at pos using "swap and pop"
        // iterators are invalidated
        void remove(std::size_t pos) noexcept {
            const std::size_t current_size{ size() };
            FSTLOG_ASSERT(pos < current_size);
            if (pos == current_size - 1) {
                pop_back();
            }
            else {
                if constexpr (std::is_trivially_assignable_v<T,T> 
                    && std::is_nothrow_assignable_v<T,T>)
                {
                    *(begin_ + pos) = *(--end_);
                }
                else {
                    std::swap(*(begin_ + pos), *(--end_));
                }
                std::destroy_at(end_);
            }
        }

        // iterators are invalidated
        void clear() noexcept {
            if (begin_ == nullptr) return;
            if constexpr (!std::is_trivially_destructible_v<T>) {
                std::destroy(begin_, end_);
            }
            end_ = begin_;
        }
        
        T* data() noexcept {
            return begin_;
        }

        const T* data() const noexcept {
            return begin_;
        }

        T& operator[](std::size_t ind) noexcept {
            FSTLOG_ASSERT(ind < size());
            return begin_[ind];
        }

        T const& operator[](std::size_t ind) const noexcept {
            FSTLOG_ASSERT(ind < size());
            return begin_[ind];
        }

        [[nodiscard]] std::size_t size() const noexcept {
            if (begin_ == nullptr) {
                FSTLOG_ASSERT(end_ == nullptr && cap_end_ == nullptr);
                return 0;
            }
            FSTLOG_ASSERT(end_ >= begin_ && cap_end_ >= end_);
            return static_cast<std::size_t>(end_ - begin_);
        }

        [[nodiscard]] std::size_t capacity() const noexcept {
            if (begin_ == nullptr) {
                FSTLOG_ASSERT(end_ == nullptr && cap_end_ == nullptr);
                return 0;
            }
            FSTLOG_ASSERT(end_ >= begin_ && cap_end_ >= end_);
            return static_cast<std::size_t>(cap_end_ - begin_);
        }

        [[nodiscard]] std::size_t free() const noexcept {
            if (begin_ == nullptr) {
                FSTLOG_ASSERT(end_ == nullptr && cap_end_ == nullptr);
                return 0;
            }
            FSTLOG_ASSERT(end_ >= begin_ && cap_end_ >= end_);
            return static_cast<std::size_t>(cap_end_ - end_);
        }

        [[nodiscard]] bool empty() const noexcept {
            return end_ == begin_;
        }

        [[nodiscard]] bool full() const noexcept {
            return end_ == cap_end_;
        }

        using iterator = T*;
        using const_iterator = T const*;

        iterator begin() noexcept { 
            return begin_; 
        }

        const_iterator begin() const noexcept {
            return begin_;
        }

        iterator end() noexcept { 
            return end_; 
        }

        const_iterator end() const noexcept {
            return end_;
        }

        // iterators are invalidated
        bool grow(std::size_t minimum_growth = 0) noexcept {
            const std::size_t old_capacity{ capacity() };
            if ((std::numeric_limits<std::size_t>::max)() - old_capacity < minimum_growth) {
                return false;
            }
            const std::size_t min_new_capacity = old_capacity + minimum_growth;
            std::size_t growth = old_capacity / 2;
            if (growth < 8) growth = 8;
            if ((std::numeric_limits<std::size_t>::max)() - old_capacity < growth) {
                growth = 0;
            }
            std::size_t new_capacity = old_capacity + growth;
            if (new_capacity < min_new_capacity) {
                new_capacity = min_new_capacity;
            }
            return reallocate_memory(new_capacity);
        }
    
        // not shrink_to_fit, it shrinks to size() * 1.5 to avoid immediate reallocation
        // iterators are invalidated
        bool shrink(std::size_t minimum_capacity = 0) noexcept {
            const std::size_t old_capacity{ capacity() };
            const std::size_t curr_size = size();
            std::size_t excess = curr_size / 2;
            if (excess < 8) excess = 8;
            if ((std::numeric_limits<std::size_t>::max)() - curr_size < excess) {
                excess = 0;
            }
            std::size_t new_capacity = curr_size + excess;
            if (new_capacity < minimum_capacity) {
                new_capacity = minimum_capacity;
            }
            // we do not grow
            if (old_capacity <= new_capacity) {
                return true;
            }
            return reallocate_memory(new_capacity);
        }

        // iterators are invalidated
        template<bool value_init = true>
        bool resize(std::size_t new_size) noexcept {
            const std::size_t current_size = size();
            if (new_size == current_size) return true;
            if (new_size < current_size) {
                if constexpr (!std::is_trivially_destructible_v<T>) {
                    std::destroy(begin_ + new_size, begin_ + current_size);
                }
            }
            else {
                const std::size_t inc = new_size - current_size;
                if (capacity() < new_size) {
                    if (!grow(inc)) return false;
                }
                if constexpr (value_init) {
                    std::uninitialized_value_construct_n(end_, inc);
                }
                else {
                    std::uninitialized_default_construct_n(end_, inc);
                }
            }
            end_ = begin_ + new_size;
            return true;
        }

        // resize the array, without value initializing (no 0 fill)
        // iterators are invalidated
        bool resize_uninitialized(std::size_t new_size) noexcept {
            return resize<false>(new_size);
        }

        // iterators are invalidated
        void pop_back() noexcept {
            FSTLOG_ASSERT(!empty());
            std::destroy_at(--end_);
        }

        // invalidates iterators
        bool write(T const* source_data, std::size_t source_data_len) noexcept {
            FSTLOG_ASSERT(source_data != nullptr);
            FSTLOG_ASSERT(source_data_len != 0);
            const std::size_t free_space = free();
            if (free_space < source_data_len) {
                if(!grow(source_data_len - free_space)){
                    return false;
                }
            }
            std::uninitialized_copy_n(source_data, source_data_len, end_);
            end_ += source_data_len;
            return true;
        }

        memory_resource* get_memory_resource() const noexcept {
            return memory_resource_;
        }

    private:

        // iterators are invalidated
        // does not change elements, used to manage free space
        bool reallocate_memory(std::size_t new_capacity) noexcept {
            const std::size_t current_size{ size() };
            FSTLOG_ASSERT(new_capacity >= current_size && "Reallocation can not cause data loss!");
            FSTLOG_ASSERT(new_capacity != 0 && "Can not allocate 0 size memory!");
            if (new_capacity < current_size || new_capacity == 0) {
                return false;
            }
            if (new_capacity == current_size) {
                return true;
            }
            T* new_memory = nothrow_allocate<T>(memory_resource_, new_capacity);
            if (new_memory == nullptr) {
                return false;
            }
            if (begin_ != nullptr) {
                std::uninitialized_move(begin_, end_, new_memory);
                if constexpr (!std::is_trivially_destructible_v<T>) {
                    std::destroy(begin_, end_);
                }
                nothrow_deallocate(begin_, memory_resource_, capacity());
            }
            begin_ = new_memory;
            cap_end_ = new_memory + new_capacity;
            end_ = begin_ + current_size;
            return true;
        }

        static_assert(std::is_default_constructible_v<T>, "Type must be default constructible");
        static_assert(std::is_nothrow_move_constructible_v<T>, "Value type bad!");
        static_assert(std::is_nothrow_constructible_v<T>, "Value type bad!");
        static_assert(std::is_nothrow_copy_constructible_v<T>, "Value type bad!");
        static_assert(std::is_nothrow_swappable_v<T>, "Value type bad!");
        static_assert(!std::is_array_v<T>, "Value type bad!");

        T* begin_{ nullptr };
        T* end_{ nullptr };
        T* cap_end_{ nullptr };
        memory_resource* const memory_resource_;
    };

}
