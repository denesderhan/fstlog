//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cassert>
#include <cstddef>
#include <cstring>
#include <new>
#include <utility>

#include <detail/constants_src.hpp>
#include <fstlog/detail/memory_resource.hpp>
#include <detail/nothrow_allocate.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {

    class dyn_buffer {
    public:

        explicit dyn_buffer(memory_resource* resource = fstlog::get_default_resource()) noexcept
            : memory_resource_{ resource } {}
        explicit dyn_buffer(std::size_t size) noexcept
            : dyn_buffer(size, fstlog::get_default_resource()) {}
        dyn_buffer(std::size_t size, memory_resource* resource) noexcept
            : memory_resource_{ resource } 
        {
            grow(size);
        }

        ~dyn_buffer() noexcept {
            if (begin_ != nullptr) {
                aligned_nothrow_deallocate(
                    begin_, 
                    memory_resource_, 
                    capacity(), 
                    constants::cache_ls_nosharing);
            }
        }

        dyn_buffer(dyn_buffer const&) = delete;
        dyn_buffer& operator=(dyn_buffer const&) = delete;
        dyn_buffer(dyn_buffer&&) = delete;
        dyn_buffer& operator=(dyn_buffer&&) = delete;
                        
        bool write(unsigned char const* data, std::size_t byte_num) noexcept {
            if (data == nullptr || byte_num == 0) return true;
            if (capacity() - size() < byte_num) {
                grow(byte_num - (capacity() - size()));
                if (capacity() - size() < byte_num) {
                    return false;
                }
            }
            std::memcpy(end_, data, byte_num);
            end_ += byte_num;
            return true;
        }

        void clear() noexcept {
            end_ = begin_;
        }
        
        unsigned char& operator[](std::size_t ind) noexcept {
            FSTLOG_ASSERT(ind < size());
            return begin_[ind];
        }
        unsigned char const& operator[](std::size_t ind) const noexcept {
            FSTLOG_ASSERT(ind < size());
            return begin_[ind];
        }

        [[nodiscard]] std::size_t size() const noexcept {
            return end_ - begin_;
        }
        [[nodiscard]] std::size_t capacity() const noexcept {
            return cap_end_ - begin_;
        }
        [[nodiscard]] bool empty() const noexcept {
            return end_ == begin_;
        }

        unsigned char* end() noexcept { 
            return end_; 
        }
        unsigned char* data() const noexcept {
            return begin_;
        }

        void grow(std::size_t inc = 0) noexcept {
            const auto old_capacity{ capacity() };
            auto min_new_capacity = old_capacity + inc;
            //checking for overflow
            if (min_new_capacity < old_capacity) return;
            auto new_capacity = old_capacity < 2 ? 2 : old_capacity + old_capacity / 2;
            if (new_capacity < min_new_capacity) new_capacity = min_new_capacity;
            reallocate_memory(new_capacity);
        }
    
        void shrink(std::size_t min_cap) {
            const auto cur_capacity{ capacity() };
            if (cur_capacity <= min_cap) return;
            const auto cur_size{ size() };
            auto new_capacity = cur_size * 2;
            if (new_capacity < min_cap) new_capacity = min_cap;
            //cur_cap - new_cap <= min_cap not worth to realloc
            if (new_capacity >= cur_capacity - min_cap) return;
            reallocate_memory(new_capacity);
        }

        memory_resource* get_memory_resource() const noexcept {
            return memory_resource_;
        }

    private:

        void reallocate_memory(std::size_t new_capacity) noexcept {
            const auto cur_size{ size() };
            if (new_capacity < cur_size || new_capacity == 0) return;
            unsigned char* new_memory = static_cast<unsigned char*>(
                aligned_nothrow_allocate(memory_resource_, new_capacity, constants::cache_ls_nosharing));
            if (new_memory != nullptr) {
                const auto data_size(size());
                if (begin_ != nullptr) {
                    if (data_size != 0) std::memcpy(new_memory, begin_, data_size);
                    aligned_nothrow_deallocate(begin_, memory_resource_, capacity(), constants::cache_ls_nosharing);
                }
                begin_ = new_memory;
                cap_end_ = new_memory + new_capacity;
                end_ = begin_ + data_size;
            }
        }
        
        unsigned char* begin_{ nullptr };
        unsigned char* end_{ nullptr };
        unsigned char* cap_end_{ nullptr };
        memory_resource* const memory_resource_;
    };

}
