//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>

#include <detail/byte_span.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/small_string.hpp>

namespace fstlog {
	template<std::size_t class_size_>
	class alignas(class_size_) time_string {	
	public:
		time_string() noexcept = default;
		template<typename T>
		time_string(byte_span<T> string, unsigned char sec_pos = 255) noexcept
			: second_pos_{ sec_pos },
			time_str_{reinterpret_cast<const char*>(string.data()),
				string.size_bytes()}
			
		{
			static_assert(sizeof(T) == 1, "Only utf8 allowed!");
			FSTLOG_ASSERT(string.size_bytes() <= capacity());
			FSTLOG_ASSERT(sec_pos <= string.size_bytes() || sec_pos == 255);
		}
		
		bool has_second() const noexcept {
			return second_pos_ < 255;
		}
		static constexpr std::size_t capacity() noexcept {
			return small_string<class_size_ - 1>::capacity();
		}
		
		const char* data() const noexcept {
			return time_str_.data();
		}

		std::size_t size() const noexcept {
			return time_str_.size();
		}

		std::size_t second_pos() const noexcept {
			return second_pos_;
		}

	private:
		unsigned char second_pos_{ 255 };
		small_string<class_size_ - 1> time_str_;
		static_assert(class_size_ == 32 || class_size_ == 64 
			|| class_size_ == 128 || class_size_ == 256,
			"Invalid class size!");
	};
#ifdef FSTLOG_DEBUG
	static_assert(sizeof(time_string<32>) == 32 && sizeof(time_string<64>) == 64
		&& sizeof(time_string<128>) == 128 && sizeof(time_string<256>) == 256);
#endif
}
