//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>
#include <cstring>
#include <thread>
#pragma intrinsic(memcpy)

namespace fstlog {
	namespace this_thread {
		inline std::uint32_t get_id() noexcept {
			const auto thread_id{ std::this_thread::get_id() };
			if constexpr (sizeof(thread_id) <= sizeof(std::uint32_t)) {
				std::uint32_t thr_id_num{ 0 };
				memcpy(&thr_id_num, &thread_id, sizeof(thread_id));
				return thr_id_num;
			}
			else {
				std::uint64_t thr_id_num{ 0 };
				if constexpr (sizeof(thread_id) <= sizeof(std::uint64_t)) {
					memcpy(&thr_id_num, &thread_id, sizeof(thread_id));
				}
				else {
					memcpy(&thr_id_num, &thread_id, sizeof(thr_id_num));
				}
				return static_cast<std::uint32_t>((thr_id_num >> 32) ^ (thr_id_num & 0xffffffff));
			}
		}
	}
}
