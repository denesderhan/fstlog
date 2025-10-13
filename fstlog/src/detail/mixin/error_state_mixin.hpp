//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <type_traits>

#include <detail/error.hpp>

namespace fstlog {
    template<typename L>
    class error_state_mixin : public L {
    public:
        using memory_resource_type = typename L::memory_resource_type;

        explicit error_state_mixin(memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<L, memory_resource_type*>) 
            : L(resource) {}

        error_state_mixin(const error_state_mixin& other) noexcept(
            noexcept(other.get_memory_resource())
            && std::is_nothrow_constructible_v<
                error_state_mixin,
                const error_state_mixin&,
                memory_resource_type*>)
            : error_state_mixin(other, other.get_memory_resource()) {}
        error_state_mixin(const error_state_mixin& other, memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<
                L,
                const error_state_mixin&,
                memory_resource_type*>)
            : L(other, resource) {}

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
