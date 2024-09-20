//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#ifndef NDEBUG
#include <fstlog/detail/is_pow2.hpp>
#include <limits>
#endif
namespace fstlog {
	template<typename T>
	struct log2_size {
		static constexpr unsigned char value = []{
#ifndef NDEBUG
			static_assert(std::numeric_limits<unsigned char>::digits == 8);
			static_assert(sizeof(T) > 0 && sizeof(T) <= 128,
				"Type size not in range!");
			static_assert(is_pow2(sizeof(T)),
				"Type size not power of 2!");
#endif
			unsigned char s = static_cast<unsigned char>(sizeof(T));
			unsigned char log2s = 0;
			while(true) {
				s >>= 1;
				if (s == 0 ) break;
				log2s++;
			};
			return log2s;
		}();
	};
}
