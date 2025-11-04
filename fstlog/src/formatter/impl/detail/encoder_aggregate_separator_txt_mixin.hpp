//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/aggregate_type.hpp>
#include <fstlog/detail/types.hpp>
#include <fstlog/detail/error_code.hpp>

namespace fstlog {
    template<typename L>
    class encoder_aggregate_separator_txt_mixin : public L
    {
    public:
        void encode_aggregate_start(
            [[maybe_unused]] aggregate_type type, 
            [[maybe_unused]] msg_counter element_number) noexcept 
        {
            if (this->output_has_space()) {
                *this->output_ptr() = '[';
                this->advance_output_unchecked(1);
            }
            else {
                this->set_error(__FILE__, __LINE__, error_code::buff_full);
            }
        }

        void encode_aggregate_element_separator() noexcept {
            if (this->output_has_space(2)) {
                const auto o_ptr{ this->output_ptr() };
                *o_ptr = ',';
                *(o_ptr + 1) = ' ';
                this->advance_output_unchecked(2);
            }
            else {
                this->set_error(__FILE__, __LINE__, error_code::buff_full);
            }
        }

        void encode_aggregate_stop() noexcept {
            if (this->output_has_space()) {
                *this->output_ptr() = ']';
                this->advance_output_unchecked(1);
            }
            else {
                this->set_error(__FILE__, __LINE__, error_code::buff_full);
            }
        }
    };
}
