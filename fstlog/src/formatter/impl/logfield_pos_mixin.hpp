//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>

#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/log_metaargs.hpp>
#include <fstlog/detail/ut_cast.hpp>
#include <formatter/impl/detail/logfield.hpp>

namespace fstlog {
    template<typename L>
    class logfield_pos_mixin : public L
    {
    public:
        logfield_pos_mixin() noexcept = default;

        logfield_pos_mixin(const logfield_pos_mixin&) = delete;
        logfield_pos_mixin& operator=(const logfield_pos_mixin&) = delete;
        logfield_pos_mixin(logfield_pos_mixin&&) = delete;
        logfield_pos_mixin& operator=(logfield_pos_mixin&&) = delete;
        
        ~logfield_pos_mixin() = default;
        
        // Call this when input_ptr is at the end of header!
        void init_logfields() noexcept {
            field_pos_.fill(nullptr);
            
            constexpr std::array<std::pair<log_metaargs,logfield>, 5> lut {{
                    {log_metaargs::Logger, logfield::Logger},
                    {log_metaargs::Thread, logfield::Thread},
                    {log_metaargs::File, logfield::File},
                    {log_metaargs::Line, logfield::Line},
                    {log_metaargs::Function, logfield::Function}
            }};

            const auto flags{ this->header_flags() };

            if (flags != 0) {
                for (std::pair<log_metaargs, logfield> inds : lut) {
                    if ((flags & ut_cast(inds.first)))
                        set_pos_and_skip(inds.second);
                }
            }

            //message is mandatory
            set_pos_and_skip(logfield::Message);
            FSTLOG_ASSERT(logfield::Args <= logfield_last);
            if (!this->end_of_input_data())
                field_pos_[ut_cast(logfield::Args)] = this->input_ptr();
        }

        bool seek_field(logfield field) noexcept {
            FSTLOG_ASSERT(field <= logfield_last);
            auto pos = field_pos_[ut_cast(field)];
            if (pos != nullptr) {
                this->set_input_ptr_unchecked(pos);
                return true;
            }
            return false;
        }

    private:
        void set_pos_and_skip(logfield field) noexcept {
            FSTLOG_ASSERT(field <= logfield_last);
            const auto pos{ this->input_ptr() };
            this->skip_argument();
            if (!this->has_error())
                field_pos_[ut_cast(field)] = pos;
            else field_pos_[ut_cast(field)] = nullptr;
        }
        
        std::array<const unsigned char*, ut_cast(logfield_last) + 1> field_pos_{ nullptr };
    };
}
