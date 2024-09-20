//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once

//assertions are used only in Debug builds and only if FSTLOG_DEBUG is defined 
//(intended for debugging fstlog library code)
#ifndef FSTLOG_ASSERT
	#ifdef FSTLOG_DEBUG
		#include <cassert>
		#define FSTLOG_ASSERT(x) assert(x)
	#else
		#define FSTLOG_ASSERT(x) ((void)0)
	#endif
#endif
