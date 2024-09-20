//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/fstlog_assert.hpp>
#include <detail/buffer_operation_result.hpp>
#include <detail/safe_reinterpret_cast.hpp>

namespace fstlog {

	inline constexpr int utf8_bytes(unsigned char ch) noexcept {
        if (ch <= 127) return 1;
        if ((ch & 0b11100000) == 0b11000000) return 2;
        if ((ch & 0b11110000) == 0b11100000) return 3;
        if ((ch & 0b11111000) == 0b11110000) return 4;
        return 0;
    }

	//if invalid returns 0, if valid returns the byte number
    inline constexpr int valid_utf8(
		const unsigned char* data, 
		const unsigned char* end) noexcept
	{
        FSTLOG_ASSERT(data != nullptr && end >= data);
		const auto byte_num = utf8_bytes(*data);
		if (byte_num != 0 && end - data >= byte_num) {
			data++;
			for (int i = 0; i < byte_num - 1; i++)
				if ((*data++ & 0b11000000) != 0b10000000) return 0;
			return byte_num;
		}
		else return 0;
    }

	inline constexpr bool printable_utf8(
		const unsigned char* data, 
		int bytes) noexcept 
	{
		FSTLOG_ASSERT(bytes > 0 && bytes < 5);
		if (bytes == 1) {
			const auto cp{ *data };
			if (cp >= 32 && cp <= 126) return true;
			else return false;
		}
		else if (bytes == 2) {
			std::uint32_t cp1 = ((*data) & std::uint32_t{ 0x1F }) << 6;
			std::uint32_t cp0 = (*(data + 1)) & std::uint32_t{ 0x3F };
			std::uint32_t cp = cp0 | cp1;
			if (cp > 159) return true;
			else return false;
		}
		else return true;
	}

	inline void sanitize_utf8_str(
		unsigned char* data, 
		const unsigned char* end ) noexcept
	{
		auto pos{ data };
		while (pos < end) {
			const auto ch{*pos};
			//printable ASCII (valid 1 byte excl. control)
			if (ch >= 32 && ch <= 126) {
				pos++;
			}
			else {
				const auto byte_num{ valid_utf8(pos, end) };
				//invalid or control
				if (byte_num <= 1) {
					*pos++ = '_';
				}
				else if (byte_num == 2) {
					std::uint32_t cp1 = (ch & std::uint32_t{ 0x1F }) << 6;
					std::uint32_t cp0 = (*(pos + 1)) & std::uint32_t{ 0x3F };
					std::uint32_t cp = cp0 | cp1;
					if (cp <= 159) {
						*pos = 0xc2;
						*(pos + 1) = 0xa1;
					}
					pos += 2;
				}
				else {
					pos += byte_num;
				}
			}
		}
	}

	template<typename T>
	inline constexpr std::size_t charnum_utf8(const T* begin, const T* end) noexcept {
		if (begin == nullptr) return 0;
		auto pos{ begin };
		std::size_t num{ 0 };
		while (pos < end) {
			static_assert(sizeof(T) == 1);
			auto bytes = valid_utf8(
				safe_reinterpret_cast<const unsigned char*>(pos),
				safe_reinterpret_cast<const unsigned char*>(end));
			if (bytes == 0) bytes = 1;
			num++;
			pos += bytes;
		}
		return num;
	}

	template<typename T>
	inline T* utf8_str_trim(T* begin, T* end, std::size_t& wanted_num) noexcept	{
		if (begin == nullptr) {
			wanted_num = 0;
			return nullptr;
		}
		auto pos{ begin };
		std::size_t trimmed_num{ 0 };
		while (pos < end && trimmed_num != wanted_num) {
			static_assert(sizeof(T) == 1);
			auto bytes = valid_utf8(
				safe_reinterpret_cast<const unsigned char*>(pos),
				safe_reinterpret_cast<const unsigned char*>(end));
			if (bytes == 0) bytes = 1;
			trimmed_num++;
			pos += bytes;
		}
		wanted_num = trimmed_num;
		return pos;
	}

	template<typename I, typename O>
	inline detail::buffer_operation_result<O> utf8cpy(
		const unsigned char*& input,
		const unsigned char* input_end,
		O* dest,
		const O* dest_end,
		std::size_t& char_num) noexcept
	{
		if (input == nullptr) {
			return detail::buffer_operation_result<O>{ dest, error_code::none };
		}
		static_assert(sizeof(I) == 1);
		static_assert(std::is_integral_v<O> &&
			(std::numeric_limits<O>::max)() >= (std::numeric_limits<unsigned char>::max)(),
			"Invalid type!");
		std::size_t num{ 0 };
		while (input < input_end && num != char_num) {
			auto bytes = valid_utf8(input, input_end);
			if (bytes == 0) {
				return detail::buffer_operation_result<O>{
					dest, error_code::input_contract_violation };
			}

			if (dest_end - dest >= bytes) {
				num++;
				while (bytes-- != 0) { 
					*dest++ = *input++;
				};
			}
			else {
				char_num = num;
				return detail::buffer_operation_result<O>{
					dest, error_code::no_space_in_buffer };
			}
		}
		char_num = num;
		return detail::buffer_operation_result<O>{ dest, error_code::none };
	}
}
