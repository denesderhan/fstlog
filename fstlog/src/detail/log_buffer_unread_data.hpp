//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>

namespace fstlog {
	struct log_buffer_unread_data {
		unsigned char const* pos1{ nullptr };
		unsigned char const* pos2{ nullptr };
		std::uint32_t size1{ 0 };
		std::uint32_t size2{ 0 };
	};
}
