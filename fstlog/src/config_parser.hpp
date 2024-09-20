//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#ifndef FSTLOG_CONTAINER_NESTING_DEPTH
#define FSTLOG_CONTAINER_NESTING_DEPTH 10
#endif
namespace fstlog {
	namespace config {
		//Depth of nested container trees that a core will go down while parsing an internal log message
		inline constexpr int max_parser_tree_depth{ FSTLOG_CONTAINER_NESTING_DEPTH };
	}
}
