//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#ifdef FSTLOG_DEBUG

#include <cstdint>
#include <type_traits>

namespace fstlog {

	template<typename T>
	struct stripped_type {
		using type = std::remove_cv_t<std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<T>>>>;
	};

	template <typename T>
	using stripped_type_t = typename stripped_type<T>::type;

	//Invalid use (default fallback)
	template<typename To, typename Fro, typename ...X>
	constexpr auto reinterpret_wrapper([[maybe_unused]] Fro&& from, X...) noexcept {
		static_assert(!sizeof(To), "Error, Invalid use of reinterpret_cast!");
		return To{};
	};

	//1) An expression of integral, (not used here: enumeration, pointer, or pointer-to-member type) 
	// can be converted to its own type. 
	// The resulting value is the same as the value of expression.
	template<typename To, typename Fro,
	std::enable_if_t<
		std::is_integral_v<stripped_type_t<To>> 
		&& !std::is_pointer_v<To>
		&& std::is_same_v<To, std::remove_reference_t<Fro>>
	>* = nullptr>
	constexpr auto reinterpret_wrapper(Fro&& from) noexcept {
		return reinterpret_cast<To>(std::forward<Fro>(from));
	};

	//2) A pointer can be converted to any integral type 
	// large enough to hold all values of its type 
	// (e.g. to std::uintptr_t).
	template<typename To, typename Fro,
	std::enable_if_t<
		std::is_same_v<To, std::uintptr_t>
		&& std::is_pointer_v<std::remove_reference_t<Fro>>
	>* = nullptr>
	constexpr auto reinterpret_wrapper(Fro&& from) noexcept {
		return reinterpret_cast<To>(std::forward<Fro>(from));
	};
	
	template<typename S, typename U>
	struct sign_unsign_is_same_base {
		static constexpr bool value =
			(std::is_same_v<signed char,S> && std::is_same_v<unsigned char, U>)
			|| (std::is_same_v<signed short, S> && std::is_same_v<unsigned short, U>)
			|| (std::is_same_v<signed int, S> && std::is_same_v<unsigned int, U>)
			|| (std::is_same_v<signed long, S> && std::is_same_v<unsigned long, U>)
			|| (std::is_same_v<signed long long, S> && std::is_same_v<unsigned long long, U>);
	};

	//1) An expression of (not used here: integral, enumeration, ) pointer, or pointer-to-member type 
	// can be converted to its own type.
	//Aliasig: AliasedType is the (possibly cv-qualified) signed or unsigned variant of DynamicType. 
	template<typename To, typename Fro,
	std::enable_if_t<
		std::is_pointer_v<To>
		&& std::is_pointer_v<std::remove_reference_t<Fro>>
		&& (
			std::is_same_v<stripped_type_t<To>, stripped_type_t<Fro>>
			|| sign_unsign_is_same_base<stripped_type_t<To>, stripped_type_t<Fro>>::value
			|| sign_unsign_is_same_base<stripped_type_t<Fro>, stripped_type_t<To>>::value)
	>* = nullptr>
	constexpr auto reinterpret_wrapper(Fro&& from) noexcept {
		return reinterpret_cast<To>(std::forward<Fro>(from));
	};

	//AliasedType is std::byte, char, or unsigned char: this permits examination 
	// of the object representation of any object as an array of bytes. 
	template<typename To, typename Fro,
		std::enable_if_t<
		std::is_pointer_v<To>
		&& std::is_pointer_v<std::remove_reference_t<Fro>>
		&& !std::is_same_v<stripped_type_t<To>, stripped_type_t<Fro>>
		&& (
			std::is_same_v<stripped_type_t<To>, char>
			|| (std::is_same_v<stripped_type_t<To>, unsigned char>
				&& !std::is_same_v<stripped_type_t<Fro>, signed char>)
			|| std::is_same_v<stripped_type_t<To>, std::byte>)
	>* = nullptr>
	constexpr auto reinterpret_wrapper(Fro&& from) noexcept {
		return reinterpret_cast<To>(std::forward<Fro>(from));
	};

	template<typename To, typename Fro>
	constexpr auto safe_reinterpret_cast(Fro&& from) noexcept {
		return reinterpret_wrapper<To>(std::forward<Fro>(from));
	};
}
#else
namespace fstlog {
	template<typename To, typename Fro>
	constexpr auto safe_reinterpret_cast(Fro&& from) noexcept {
		return reinterpret_cast<To>(std::forward<Fro>(from));
	};
}
#endif
