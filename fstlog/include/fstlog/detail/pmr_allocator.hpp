//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <memory_resource>

namespace fstlog {
	using fstlog_allocator = std::pmr::polymorphic_allocator<unsigned char>;
}
