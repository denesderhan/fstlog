//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#ifdef FSTLOG_DEBUG

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace fstlog {
	template <typename T>
	using stripped_type_t = 
		std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;
	
	// Enforces safe casts in debug mode; in release mode, reinterpret_cast is unrestricted.
	template<typename T, typename F>
	[[nodiscard]] constexpr auto safe_reinterpret_cast(F&& from) noexcept -> T {
		if constexpr (
			std::is_pointer_v<std::remove_reference_t<F>> 
			&& std::is_same_v<T, std::uintptr_t>) 
		{
			// A pointer can be converted to any integral type 
			// large enough to hold all values of its type (e.g. to std::uintptr_t).
			return reinterpret_cast<T>(std::forward<F>(from));
		}
		else if constexpr (
			std::is_pointer_v<std::remove_reference_t<F>> 
			&& std::is_pointer_v<std::remove_reference_t<T>> 
			&& (std::is_same_v<stripped_type_t<T>, char>
				|| std::is_same_v<stripped_type_t<T>, unsigned char>
				|| std::is_same_v<stripped_type_t<T>, std::byte>))
		{
			// AliasedType is char, unsigned char or std::byte: this permits examination 
			// of the object representation of any object as an array of bytes. 
			return reinterpret_cast<T>(std::forward<F>(from));
		}
		else {
			// All other use of reinterpret_cast<>() is forbidden to prevent undefined behaviour.
			static_assert(!sizeof(T), "Error, forbidden use of reinterpret_cast!");
			return T{};
		}
	}
}
#else
namespace fstlog {
	// reinterpret_cast is used without any restrictions, potentially leading to undefined behavior.
	// Compile with #define FSTLOG_DEBUG to use the restricted version.
	template<typename T, typename F>
	[[nodiscard]] constexpr auto safe_reinterpret_cast(F&& from) noexcept -> T {
		return reinterpret_cast<T>(std::forward<F>(from));
	};
}
#endif
