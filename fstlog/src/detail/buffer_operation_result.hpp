//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <detail/error_code.hpp>

namespace fstlog {
	namespace detail{
		template<typename T = unsigned char>
		struct buffer_operation_result {
			T* ptr;
			error_code ec;
		};
	}
}
