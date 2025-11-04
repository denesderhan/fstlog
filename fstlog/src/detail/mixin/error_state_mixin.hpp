//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <detail/error.hpp>

namespace fstlog {
    template<typename L>
    class error_state_mixin : public L {
    public:
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
