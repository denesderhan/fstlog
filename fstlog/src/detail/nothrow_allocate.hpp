//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <limits>

#include <detail/safe_reinterpret_cast.hpp>
#include <fstlog/detail/fstlog_allocator.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/is_pow2.hpp>
#include <fstlog/detail/noexceptions.hpp>

namespace fstlog {
	inline void aligned_nothrow_deallocate(
		void* obj_ptr,
		fstlog_allocator const& allocator,
		std::size_t byte_num,
		std::size_t alignment) noexcept
	{
		FSTLOG_ASSERT(obj_ptr != nullptr
			&& is_pow2(alignment)
			&& safe_reinterpret_cast<std::uintptr_t>(obj_ptr) % alignment == 0);
		allocator.resource()->deallocate(
			obj_ptr,
			byte_num,
			alignment);
	}
	
	inline void* aligned_nothrow_allocate(
		fstlog_allocator const& allocator, 
		std::size_t byte_num, 
		std::size_t alignment) noexcept
	{
		void* out_ptr{ nullptr };
		if (byte_num == 0 || !is_pow2(alignment)) {
			return out_ptr;
		}
#ifdef FSTLOG_NOEXCEPTIONS
		static_assert(noexcept(allocator.resource()->allocate(byte_num,	alignment)),
			"Exceptions disabled, but allocator.resource()->allocate() is not noexcept!");
		// allocate size with provided allocator
		out_ptr = allocator.resource()->allocate(
			byte_num,
			alignment);
		if (out_ptr != nullptr && safe_reinterpret_cast<std::uintptr_t>(out_ptr) % alignment != 0) {
			aligned_nothrow_deallocate(out_ptr, allocator, byte_num, alignment);
			out_ptr = nullptr;
		}
#else
		try {
			// allocate size with provided allocator
			out_ptr = allocator.resource()->allocate(
				byte_num,
				alignment);
			if (out_ptr != nullptr && safe_reinterpret_cast<std::uintptr_t>(out_ptr) % alignment != 0) {
				aligned_nothrow_deallocate(out_ptr, allocator, byte_num, alignment);
				out_ptr = nullptr;
			}
		}
		catch (...) {
			// if allocate throws out_ptr must be nullptr
			FSTLOG_ASSERT(out_ptr == nullptr);
		}
#endif
		return out_ptr;
	}
		
	template<class T>
	T* nothrow_allocate(
		fstlog_allocator const& allocator,
		std::size_t num = 1) noexcept
	{
		if ((std::numeric_limits<std::size_t>::max)() / sizeof(T) < num) {
			return nullptr;
		}
		else {
			return static_cast<T*>(aligned_nothrow_allocate(
				allocator, 
				sizeof(T) * num, 
				alignof(T)));
		}
	}
	
	template<class T>
	void nothrow_deallocate(
		T* obj_ptr,
		fstlog_allocator const& allocator,
		std::size_t num = 1) noexcept
	{
		FSTLOG_ASSERT((std::numeric_limits<std::size_t>::max)() / sizeof(T) >= num);
		aligned_nothrow_deallocate(obj_ptr, allocator, sizeof(T) * num, alignof(T));
	}
	
}
