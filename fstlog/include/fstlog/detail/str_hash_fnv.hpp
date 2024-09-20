//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>

#include <fstlog/detail/fnv_hash.hpp>

namespace fstlog {
		
	class str_hash_fnv
	{
	public:
		constexpr str_hash_fnv() = default;
		
		explicit constexpr str_hash_fnv(std::uint32_t hash) noexcept
			:hash_(hash){ }

		template<typename T>
		constexpr str_hash_fnv(const T* data, std::size_t size) noexcept
			: hash_(fnv_1a_32(data, size)) {}

		template<typename T, std::size_t N>
		constexpr str_hash_fnv(const T(&str)[N]) noexcept {
			static_assert(!sizeof(T), "Constructing str_hash_fnv from char arrays is forbidden, for string literals use the _hs operator!");
		}

		template<typename T>
		constexpr str_hash_fnv(std::basic_string_view<T> strv) noexcept
			: hash_(fnv_1a_32(strv.data(), strv.size())) {}

		std::uint32_t hash_{ 0 };
	};
#ifdef FSTLOG_DEBUG
	static_assert(sizeof(str_hash_fnv) == sizeof(std::uint32_t), "Error, not POD type!");
#endif

	inline namespace literals {
#ifdef __cpp_consteval
		consteval
#else
		constexpr
#endif
		str_hash_fnv operator"" _hs(const char* str, std::size_t n) noexcept
		{
			return str_hash_fnv(str, n);
		}

#ifdef __cpp_char8_t
#ifdef __cpp_consteval
		consteval
#else
		constexpr
#endif
		str_hash_fnv operator"" _hs(const char8_t* str, std::size_t n) noexcept
		{
			return str_hash_fnv(str, n);
		}
#endif
	}
}
