//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once

#ifndef FSTLOG_API
#if defined(FSTLOG_SHARED)
	#if defined(_WIN32)
		#if defined(FSTLOG_EXPORT)
			#define FSTLOG_API __declspec(dllexport)
		#else
			#define FSTLOG_API __declspec(dllimport)
		#endif
	#elif defined(__GNUC__) || defined(__clang__)
		#define FSTLOG_API __attribute__((visibility("default")))
	#else
		#define FSTLOG_API
	#endif
#else
	#define FSTLOG_API
#endif
#endif
