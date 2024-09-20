//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <type_traits>
#include <string_view>
#include <cassert>

#include <fstlog/detail/fstlog_assert.hpp>
#ifdef FSTLOG_DEBUG
#include <fstlog/detail/is_char_type.hpp>
#endif

namespace fstlog {
	template<typename T>
	constexpr auto convert_to_basic_string_view(T const& string_like) noexcept {
		if constexpr (std::is_convertible_v<T, std::string_view>) {
			const std::string_view out{ string_like };
			assert(out.data() != nullptr || out.size() == 0 && "Bad string (nullptr with non null size)!");
			return out;
		}
#ifdef __cpp_char8_t		
		else if constexpr (std::is_convertible_v<T, std::u8string_view>) {
			const std::u8string_view out{ string_like };
			assert(out.data() != nullptr || out.size() == 0 && "Bad string (nullptr with non null size)!");
			return out;
		}
#endif		
		else if constexpr (std::is_convertible_v<T, std::u16string_view>) {
			const std::u16string_view out{ string_like };
			assert(out.data() != nullptr || out.size() == 0 && "Bad string (nullptr with non null size)!");
			return out;
		}
		else if constexpr (std::is_convertible_v<T, std::u32string_view>) {
			const std::u32string_view out{ string_like };
			assert(out.data() != nullptr || out.size() == 0 && "Bad string (nullptr with non null size)!");
			return out;
		}
		else {
			static_assert(!sizeof(T), "Can not convert to basic_string_view!");
			return 0;
		}
	}

	//can not differenciate string literal from ref to char array
	//const char hi[3]{ 'a', 'b', 'c' };
	//const char(&hi2)[3] = hi;
	//static_assert(std::is_same_v<decltype("Hi"), decltype(hi2)>);
	//will convert assuming last char is 0

	template<typename T, std::size_t N>
	constexpr auto convert_to_basic_string_view(const T(&str)[N]) noexcept {
		FSTLOG_ASSERT(is_char_type_v<T>);
		assert(str != nullptr && N != 0 && "Bad string (nullptr or null size)!");
		return std::basic_string_view<T>{str, N - 1};
	}

	template<typename T>
	constexpr auto convert_to_basic_string_view(std::basic_string_view<T> const& strv) noexcept {
		assert(strv.data() != nullptr || strv.size() == 0 && "Bad string (nullptr with non null size)!");
		return strv;
	}
}
