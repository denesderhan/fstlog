//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <detail/error.hpp>

namespace fstlog {
    template<typename L>
    class error_state_mixin : public L {
    public:
        using allocator_type = typename L::allocator_type;

        error_state_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(error_state_mixin(allocator_type{})))
			: error_state_mixin(allocator_type{}) {}
		explicit error_state_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{}))) 
			: L(allocator) {}

		error_state_mixin(const error_state_mixin& other) noexcept(
			noexcept(error_state_mixin::get_allocator())
			&& noexcept(error_state_mixin(error_state_mixin{}, allocator_type{})))
            : error_state_mixin(other, other.get_allocator()) {}
        error_state_mixin(const error_state_mixin& other, allocator_type const& allocator) noexcept(
			noexcept(L(error_state_mixin{}, allocator_type{})))
            : L(other, allocator) {}

        error_state_mixin(error_state_mixin&& other) = delete;
        error_state_mixin& operator=(const error_state_mixin& rhs) = delete;
        error_state_mixin& operator=(error_state_mixin&& rhs) = delete;

        ~error_state_mixin() = default;

        bool has_error() const noexcept {
            return error_.code() != error_code::none;
        }

        void set_error(const char* file, int line, error_code err) noexcept {
			error_ = error{ file, line, err };
        }

        void clear_error() noexcept {
			error_ = error{};
        }

        error get_error() const noexcept {
            return error_;
        }

    private:
        error error_;
    };
}
