//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <mutex>

#include <detail/byte_span.hpp>

namespace fstlog {
    template<class L>
    class output_locked_mixin
        : public L
    {
    public:
		using allocator_type = typename L::allocator_type;

		output_locked_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(output_locked_mixin(allocator_type{})))
			: output_locked_mixin(allocator_type{}) {}
		explicit output_locked_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
			: L(allocator) {}

		output_locked_mixin(const output_locked_mixin& other) = delete;
		output_locked_mixin(output_locked_mixin&& other) = delete;
		output_locked_mixin& operator=(const output_locked_mixin& rhs) = delete;
		output_locked_mixin& operator=(output_locked_mixin&& rhs) = delete;

		~output_locked_mixin() = default;

		void write_message(buff_span_const msg) noexcept {
			std::lock_guard<decltype(L::get_mutex())> guard_instance{ L::get_mutex() };
			L::write_message(msg);
		}
		void flush() noexcept {
			std::lock_guard<decltype(L::get_mutex())> guard_instance{ L::get_mutex() };
			L::flush();
		}
		
		void try_reopen() noexcept {
			std::lock_guard<decltype(L::get_mutex())> guard_instance{ L::get_mutex() };
			L::try_reopen();
		}
    };
}
