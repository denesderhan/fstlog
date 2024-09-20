//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <type_traits>
#include <array>

#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
	template<typename T = unsigned char>
	class byte_span final {
	public:
		using element_type = T;
		using value_type = std::remove_cv_t<T>;
		using acc_type = std::conditional_t<
			std::is_const_v<T>,
			const unsigned char,
			unsigned char>;
		constexpr byte_span() noexcept = default;
				
		constexpr byte_span(acc_type* data, std::size_t byte_size) noexcept
			: data_ptr_{ data },
			bytes_{ byte_size } 
		{
			FSTLOG_ASSERT(byte_size % sizeof(T) == 0 && "Invalid size for type!");
		}

		template<typename X = std::remove_const_t<T>,
			std::enable_if_t<!std::is_same_v<X, unsigned char>>* = nullptr>
		constexpr byte_span(T* data, std::size_t size) noexcept
			: data_ptr_{ reinterpret_cast<acc_type*>(data) },
			bytes_{ size * sizeof(T) } {}

		template<std::size_t N>
		constexpr byte_span(std::array<T, N>& data) noexcept
			:data_ptr_{ reinterpret_cast<acc_type*>(data.data()) },
			bytes_{ N * sizeof(T) } {}

		template<std::size_t N>
		constexpr byte_span([[maybe_unused]] T(&str)[N]) noexcept {
			static_assert(!sizeof(T), "String literals and c arrays are forbidden!");
		}

		constexpr operator byte_span<const T>() const noexcept {
			return byte_span<const T>{ data_ptr_, bytes_ };
		}

		constexpr acc_type* data() const noexcept {
			return data_ptr_;
		}

		constexpr std::size_t size_bytes() const noexcept {
			return bytes_;
		}

		constexpr bool empty() const noexcept {
			return data_ptr_ == nullptr || bytes_ == 0;
		}

		template<typename Q = T,
			std::enable_if_t<
				std::is_const_v<Q>
				|| std::is_same_v<Q, unsigned char>>* = nullptr>
		constexpr acc_type& operator[](std::size_t byte_index) noexcept {
			FSTLOG_ASSERT(!empty() && "byte_span was empty (op. [])!");
			FSTLOG_ASSERT(byte_index < bytes_ && "Index out of bounds!");
			return *(data_ptr_ + byte_index);
		}
	private:
		acc_type* data_ptr_{ nullptr };
		std::size_t bytes_{ 0 };
	};

	using buff_span = byte_span<unsigned char>;
	using buff_span_const = byte_span<const unsigned char>;
}
