//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstdlib>
#include <new>

namespace fstlog {
	class malloc_resource {
	public:
		static void* allocate(
			std::size_t bytes,
			std::size_t alignment = alignof(std::max_align_t)) noexcept
		{
#ifdef _WIN32
			return _aligned_malloc(bytes, alignment);
#else
			return aligned_alloc(alignment, bytes);
#endif
		}

		static void deallocate(
			void* ptr,
			[[maybe_unused]] std::size_t bytes,
			[[maybe_unused]] std::size_t alignment = alignof(std::max_align_t)) noexcept
		{
#ifdef _WIN32
			_aligned_free(ptr);
#else
			free(ptr);
#endif
		}
	};
	
	template<class T>
	class malloc_allocator {
	public:
		using value_type = T;
		
		malloc_allocator() noexcept = default;
		template<class U>
		malloc_allocator(malloc_allocator<U> const&) noexcept {}

		//There are no allocate/deallocate methods
		// this class is only a wrapper for the resource
		// allocator has to be able to throw (can not return nullptr) 
		// but resource is noexcept and returns nullptr

		malloc_resource* resource() const noexcept {
			return &resource_;
		}
		
	private:
		inline static malloc_resource resource_;
	};

	template<class T>
	bool operator ==(malloc_allocator<T> const&, malloc_allocator<T> const&) noexcept {
		return true;
	}
	template<class T>
	bool operator !=(malloc_allocator<T> const&, malloc_allocator<T> const&) noexcept {
		return false;
	}

	using fstlog_allocator = malloc_allocator<unsigned char>;
}
