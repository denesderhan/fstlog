//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <new>

#include <fstlog/detail/noexceptions.hpp>
#include <detail/nothrow_allocate.hpp>

namespace fstlog {
	template<class T>
	auto make_allocated(
		fstlog_allocator const& allocator) noexcept
	{
		T* obj_ptr = nothrow_allocate<T>(allocator);
		if (obj_ptr != nullptr) {
#ifdef FSTLOG_NOEXCEPTIONS
			static_assert(noexcept(T(allocator)), "Constructor must be noexcept!");
			// storing the allocator in the object!
			obj_ptr = ::new(static_cast<void*>(obj_ptr)) T(allocator);
#else
			try {
				// storing the allocator in the object!
				obj_ptr = ::new(static_cast<void*>(obj_ptr)) T(allocator);
			}
			catch (...) {
				//constructor failed cleaning up
				nothrow_deallocate(obj_ptr, allocator);
				obj_ptr = nullptr;
			}
#endif
		}
		return typename T::wrapper_type{ obj_ptr };
	}

	template<class T, class... Args>
	auto make_allocated(
		fstlog_allocator const& allocator, 
		Args&&... args) noexcept
	{
		T* obj_ptr = nothrow_allocate<T>(allocator);
		if (obj_ptr != nullptr) {
#ifdef FSTLOG_NOEXCEPTIONS
			static_assert(noexcept(T(std::forward<Args>(args)..., allocator)),
				"Constructor must be noexcept!");
			// storing the allocator in the object!
			obj_ptr = ::new(static_cast<void*>(obj_ptr))
				T(std::forward<Args>(args)..., allocator);
#else
			try {
				// storing the allocator in the object!
				obj_ptr = ::new(static_cast<void*>(obj_ptr))
					T(std::forward<Args>(args)..., allocator);
			}
			catch (...) {
				//constructor failed cleaning up
				nothrow_deallocate(obj_ptr, allocator);
				obj_ptr = nullptr;
			}
#endif
		}
		return typename T::wrapper_type{ obj_ptr };
	}
}
