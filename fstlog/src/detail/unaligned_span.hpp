//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstring>
#include <type_traits>
#pragma intrinsic(memcpy)

#include <fstlog/detail/fstlog_assert.hpp>
#include <detail/safe_reinterpret_cast.hpp>

namespace fstlog {
	template<typename T = unsigned char>
	class unaligned_span final {
	public:
		using element_type = T;
		using value_type = std::remove_cv_t<T>;
		using size_type = std::size_t;
		using byte_type = std::conditional_t<
			std::is_const_v<T>,
			const unsigned char,
			unsigned char>;
		
		constexpr unaligned_span() noexcept = default;
		
		constexpr unaligned_span(byte_type* data, size_type byte_size) noexcept
			: data_bytes_{ data },
			byte_size_{ byte_size } 
		{
			FSTLOG_ASSERT(byte_size % sizeof(T) == 0 && "Invalid size for type!");
		}

		template<typename X = std::remove_const_t<T>,
			std::enable_if_t<!std::is_same_v<X, unsigned char>>* = nullptr>
		constexpr unaligned_span(T* data, size_type size) noexcept
			: data_bytes_{ safe_reinterpret_cast<byte_type*>(data) },
			byte_size_{ size * sizeof(T) } {}

		template<size_type N>
		constexpr unaligned_span(std::array<T, N> &data) noexcept
			:data_bytes_{ safe_reinterpret_cast<byte_type*>(data.data()) },
			byte_size_{ N * sizeof(T) } {}

		// Construction from C arrays are forbidden  
		// to prevent errors like unaligned_span("HI!") resulting in span length 4.
		template<size_type N>
		constexpr unaligned_span([[maybe_unused]] T(&str)[N]) noexcept {
			static_assert(!sizeof(T), "String literals and c arrays are forbidden!");
		}

		constexpr operator unaligned_span<const T>() const noexcept {
			return unaligned_span<const T>{ data_bytes_, byte_size_ };
		}

		constexpr byte_type* data() const noexcept {
			return data_bytes_;
		}

		constexpr size_type size() const noexcept {
			return byte_size_ / sizeof(T);
		}

		constexpr size_type size_bytes() const noexcept {
			return byte_size_;
		}

		constexpr bool empty() const noexcept {
			return data_bytes_ == nullptr || byte_size_ == 0;
		}

		struct deref;
		deref operator[](size_type index) {
			FSTLOG_ASSERT(size() > index);
			return deref(*this, index);
		}

		// proxy used for differentiating get/set
		struct deref {
			deref(unaligned_span& span, size_type index)
				: span_(span), index_(index) {}

			operator T() {
				value_type temp;
				memcpy(&temp, span_.data() + index_ * sizeof(T), sizeof(T));
				return temp;
			}

			// can not return a reference (there is no T object)
			void operator=(const T& other) {
				static_assert(!std::is_const_v<T>, "Assignment to const T not allowed!");
				memcpy(span_.data() + index_ * sizeof(T), &other, sizeof(T));
			}
			
			unaligned_span& span_;
			size_type index_;
		};

	private:
		byte_type* data_bytes_{ nullptr };
		size_type byte_size_{ 0 };

		static_assert(std::is_trivially_copyable_v<value_type>, "T must be trivially copyable!");
		static_assert(std::is_standard_layout_v<value_type>, "T must be standard layout!");
	};

	using byte_span = unaligned_span<unsigned char>;
	using byte_span_const = unaligned_span<const unsigned char>;
}
