//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#ifdef FSTLOG_DEBUG

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace fstlog {

	template<typename T>
	struct stripped_type {
		using type = std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;
	};

	template <typename T>
	using stripped_type_t = typename stripped_type<T>::type;
	
	// All other use of reinterpret_cast<>() is forbidden to prevent undefined behaviour.
	template<typename T, typename F>
	constexpr auto reinterpret_wrapper([[maybe_unused]] F&& from) noexcept {
		static_assert(!sizeof(T), "Error, forbidden use of reinterpret_cast!");
		return T{};
	};

	// An expression of integral, enumeration, pointer, or pointer-to-member type)
	// can be converted to its own type. 
	// The resulting value is the same as the value of expression.
	template<typename T, typename F,
	std::enable_if_t<
		std::is_same_v<T, std::remove_reference_t<F>>
	>* = nullptr>
	constexpr auto reinterpret_wrapper(F&& from) noexcept {
		return reinterpret_cast<T>(std::forward<F>(from));
	};

	// A pointer can be converted to any integral type 
	// large enough to hold all values of its type 
	// (e.g. to std::uintptr_t).
	template<typename T, typename F,
	std::enable_if_t<
		std::is_same_v<T, std::uintptr_t>
		&& std::is_pointer_v<std::remove_reference_t<F>>
	>* = nullptr>
	constexpr auto reinterpret_wrapper(F&& from) noexcept {
		return reinterpret_cast<T>(std::forward<F>(from));
	};

	// AliasedType is char, unsigned char or std::byte: this permits examination 
	// of the object representation of any object as an array of bytes. 
	template<typename T, typename F,
		std::enable_if_t<
			std::is_pointer_v<T> 
			&& std::is_pointer_v<std::remove_reference_t<F>>
				// safe_reinterpret_cast<char*>
			&& ((std::is_same_v<stripped_type_t<T>, char> 
					&& !std::is_same_v<stripped_type_t<F>, char>)
				// safe_reinterpret_cast<unsigned char*>
				|| (std::is_same_v<stripped_type_t<T>, unsigned char> 
					&& !std::is_same_v<stripped_type_t<F>, unsigned char> 
					&& !std::is_same_v<stripped_type_t<F>, signed char>)
				// safe_reinterpret_cast<std::byte*>
				|| (std::is_same_v<stripped_type_t<T>, std::byte> 
					&& !std::is_same_v<stripped_type_t<F>, std::byte>))
		>* = nullptr>
	constexpr auto reinterpret_wrapper(F&& from) noexcept {
		return reinterpret_cast<T>(std::forward<F>(from));
	};
		
	template<typename T, typename F>
	constexpr auto safe_reinterpret_cast(F&& from) noexcept {
		return reinterpret_wrapper<T>(std::forward<F>(from));
	};
}
#else
namespace fstlog {
	template<typename T, typename F>
	constexpr auto safe_reinterpret_cast(F&& from) noexcept {
		return reinterpret_cast<T>(std::forward<F>(from));
	};
}
#endif
