//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>

#include <fstlog/detail/fstlog_ex.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
	namespace detail {
        template<typename T>
        class checked_iterator
        {
        public:
            using difference_type  = std::ptrdiff_t;

            checked_iterator(T* start, T* end)
                : current_(start), end_(end)
			{
				FSTLOG_ASSERT(end >= start && "Invalid pointers!");
			}

            T& operator*() {
				if (current_ >= end_) throw fstlog_ex("Iterator out of bounds!");
				return *current_;
            }

            //pre increment
            checked_iterator& operator++() {
                current_++;
                return *this;
            }

            //post increment
            checked_iterator operator++(int) {
                checked_iterator temp = *this;
                ++*this;
                return temp;
            }

            T* get_ptr() const noexcept {
                return current_;
            }

        private:
            T* current_;
            T* end_;
        };
	}
}
