//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace fstlog {
	template<typename T>
	inline constexpr std::uint32_t fnv_1a_32(
		const T* data, 
		std::size_t size, 
		std::uint32_t prev_hash = std::uint32_t{ 2166136261UL }) noexcept
	{
		static_assert(std::numeric_limits<unsigned char>::digits == 8, 
			"Error, bitnum of byte != 8");
		static_assert(std::is_integral_v<T> && sizeof(T) == 1,
			"Only char and UTF8 supported");
		static_assert(std::is_unsigned_v<T> || (T(-1) & T(3)) == 3, 
			"Not twos complement!");
		std::uint32_t hash = prev_hash;
		auto char_ptr = data;
		const auto data_end = char_ptr + size;
		while (char_ptr < data_end) {
			hash ^= static_cast<std::uint32_t>(static_cast<unsigned char>(*char_ptr++));
			hash *= std::uint32_t{ 16777619UL };
		}
		return hash;
	}

	template<typename T>
	inline constexpr std::uint64_t fnv_1a_64(
		const T* data, 
		std::size_t size, 
		std::uint64_t prev_hash = std::uint64_t{ 14695981039346656037ULL }) noexcept
	{
		static_assert(std::numeric_limits<unsigned char>::digits == 8, 
			"Error, bitnum of byte != 8");
		static_assert(std::is_integral_v<T> && sizeof(T) == 1,
			"Only char and UTF8 supported");
		static_assert(std::is_unsigned_v<T> || (T(-1) & T(3)) == 3, 
			"Not twos complement!");
		std::uint64_t hash = prev_hash;
		auto char_ptr = data;
		const auto data_end = char_ptr + size;
		while (char_ptr < data_end) {
			hash ^= static_cast<std::uint64_t>(static_cast<unsigned char>(*char_ptr++));
			hash *= std::uint64_t{ 0x100000001b3ULL };
		}
		return hash;
	}
}
