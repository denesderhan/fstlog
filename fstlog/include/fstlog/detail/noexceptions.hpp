//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#ifndef FSTLOG_NOEXCEPTIONS
	#ifdef _WIN32
		#ifndef _CPPUNWIND
			#define FSTLOG_NOEXCEPTIONS
		#endif	
	#else
		#ifndef __EXCEPTIONS
			#define FSTLOG_NOEXCEPTIONS	
		#endif
	#endif
#endif
