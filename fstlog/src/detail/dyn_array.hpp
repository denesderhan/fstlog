//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cassert>
#include <cstdint>
#include <utility>
#include <new>

#include <fstlog/detail/fstlog_allocator.hpp>
#include <detail/nothrow_allocate.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
	template <typename T, class A = fstlog_allocator >
	class dyn_array {
	public:
		using value_type = T;
		using allocator_type = A;
		dyn_array() noexcept : dyn_array(allocator_type{}) {}
		explicit dyn_array(allocator_type const& allocator) noexcept
			: allocator_{ allocator } {}
		explicit dyn_array(std::size_t size) noexcept 
			: dyn_array(size, allocator_type{}) {}
		dyn_array(std::size_t size, allocator_type const& allocator) noexcept
			: allocator_{ allocator } 
		{
			grow(size);
		}

		~dyn_array() noexcept {
			const auto num_elements{ size() };
			for (std::size_t i = 0; i < num_elements; i++) {
				(*this)[i].~T();
			}
			if (begin_ != nullptr) {
				nothrow_deallocate(begin_, allocator_, capacity());
			}
		}

		dyn_array(dyn_array const&) = delete;
		dyn_array& operator=(dyn_array const&) = delete;
		dyn_array(dyn_array&&) = delete;
		dyn_array& operator=(dyn_array&&) = delete;
						
		bool try_push_back(T const& elem) noexcept {
			if (size() == capacity()) {
				grow();
				if (size() == capacity()) return false;
			}
			end_ = ::new (static_cast<void*>(end_)) T(elem);
			++end_;
			return true;
		}

		bool try_push_back(T&& elem) noexcept {
			if (size() == capacity()) {
				grow();
				if (size() == capacity()) return false;
			}
			end_ = ::new (static_cast<void*>(end_)) T(std::move(elem));
			++end_;
			return true;
		}

		//the last element is moved into the removed place!
		//iterators are invalidated
		void remove(std::size_t pos) noexcept {
			const std::size_t curr_size{ size() };
			FSTLOG_ASSERT(curr_size > 0 && pos < curr_size);
			if (pos != curr_size - 1) {
				std::swap((*this)[pos], (*this)[curr_size - 1]);
			}
			(--end_)->~T();
			shrink(64);
		}

		void clear() noexcept {
			const std::size_t num_elements{ size() };
			for (std::size_t i = 0; i < num_elements; i++) {
				(*this)[i].~T();
			}
			end_ = begin_;
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
			return end_ - begin_;
		}
		[[nodiscard]] std::size_t capacity() const noexcept {
			return cap_end_ - begin_;
		}
		[[nodiscard]] bool empty() const noexcept {
			return end_ == begin_;
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

		void grow(std::size_t inc = 0) noexcept {
			const std::size_t old_capacity{ capacity() };
			std::size_t min_new_capacity = old_capacity + inc;
			//checking for overflow
			if (min_new_capacity < old_capacity) return;
			std::size_t new_capacity = old_capacity < 2 ? 2 : old_capacity + old_capacity / 2;
			if (new_capacity < min_new_capacity) new_capacity = min_new_capacity;
			reallocate_memory(new_capacity);
		}
	
		void shrink(std::size_t min_cap) {
			const std::size_t cur_capacity{ capacity() };
			if (cur_capacity <= min_cap) return;
			const std::size_t cur_size{ size() };
			std::size_t new_capacity = cur_size * 2;
			if (new_capacity < min_cap) new_capacity = min_cap;
			//cur_cap - new_cap <= min_cap not worth to realloc
			if (new_capacity >= cur_capacity - min_cap) return;
			reallocate_memory(new_capacity);
		}

		allocator_type const& get_allocator() const noexcept {
			return allocator_;
		}

	private:

		void reallocate_memory(std::size_t new_capacity) noexcept {
			const std::size_t cur_size{ size() };
			if (new_capacity < cur_size || new_capacity == 0) return;
			T* new_memory = nothrow_allocate<T>(allocator_, new_capacity);
			if (new_memory != nullptr) {
				T* new_pos = new_memory;
				if (begin_ != nullptr) {
					T* old_pos = begin_;
					while (old_pos != end_) {
						new_pos = ::new(static_cast<void*>(new_pos)) T(std::move(*old_pos++));
						new_pos++;
					}
					nothrow_deallocate(begin_, allocator_, capacity());
				}
				begin_ = new_memory;
				cap_end_ = new_memory + new_capacity;
				end_ = new_pos;
			}
		}

		static_assert(std::is_nothrow_move_constructible_v<T>, "Value type bad!");
		static_assert(std::is_nothrow_constructible_v<T>, "Value type bad!");
		static_assert(std::is_nothrow_copy_constructible_v<T>, "Value type bad!");
		static_assert(std::is_nothrow_swappable_v<T>, "Value type bad!");
		static_assert(!std::is_array_v<T>, "Value type bad!");

		T* begin_{ nullptr };
		T* end_{ nullptr };
		T* cap_end_{ nullptr };
		const allocator_type allocator_;
	};

}
