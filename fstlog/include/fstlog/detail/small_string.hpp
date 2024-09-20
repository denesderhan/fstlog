//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstring>
#include <limits>
#include <string_view>
#pragma intrinsic(memcmp)

#include <fstlog/detail/is_pow2.hpp>

namespace fstlog {
	template<std::size_t class_size>
	class small_string {
	public:
		constexpr small_string() noexcept = default;

		constexpr small_string(const char* c_str, std::size_t length) noexcept { 
			int max_length = length < class_size - 1 ? static_cast<int>(length) : class_size - 1;
			int ind{ 0 };
			while (ind < max_length && c_str[ind] != 0) {
				string_[ind] = c_str[ind];
				ind++;
			}
			free_ = static_cast<unsigned char>(static_cast<int>(class_size) - 1 - ind);
		}

		constexpr small_string(const char* const& c_str) noexcept
			: small_string(c_str, std::char_traits<char>::length(c_str)) {}

		constexpr small_string(std::string_view str) noexcept 
			: small_string(str.data(), str.size()){}

		template<std::size_t N>
		constexpr small_string(const char(&str)[N]) noexcept 
			: small_string(str, N) {}

		constexpr char const* data() const noexcept {
			return string_;
		}
		constexpr std::size_t size() const noexcept {
			return capacity() - free_ ;
		}
		static constexpr std::size_t capacity() noexcept {
			return class_size - 1;
		}

		constexpr bool empty() const noexcept {
			return free_ == capacity();
		}

		constexpr operator std::string_view() const noexcept {
			return std::string_view{ data(), size() };
		}

		static_assert(class_size <= 256 && class_size > 1, "Invalid size!");
		static constexpr std::size_t alignment = is_pow2(class_size) ? class_size : 1;
		alignas(alignment) char string_[class_size - 1]{ 0 };
		unsigned char free_{ class_size - 1 };
	};
#ifdef FSTLOG_DEBUG
	static_assert(alignof(small_string<16>) == 16 && sizeof(small_string<16>) == 16);
	static_assert(alignof(small_string<256>) == 256 && sizeof(small_string<256>) == 256);
	static_assert(alignof(small_string<5>) == 1 && sizeof(small_string<5>) == 5);
#endif

	template<std::size_t class_size>
	constexpr inline bool operator!=(
		const small_string<class_size>& lhs, 
		const small_string<class_size>& rhs) noexcept 
	{
		static_assert(class_size == sizeof(lhs.free_) + sizeof(lhs.string_));
		return memcmp(&lhs, &rhs, class_size);
	}
	template<std::size_t class_size>
	constexpr inline bool operator==(
		const small_string<class_size>& lhs, 
		const small_string<class_size>& rhs) noexcept 
	{ 
		return !operator!=(lhs, rhs); 
	}

	template<class T>
	struct is_small_string : std::false_type {};

	template<std::size_t N>
	struct is_small_string<small_string<N>> : std::true_type {};
}
