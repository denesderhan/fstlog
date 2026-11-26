//Copyright © 2022-present, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstring>
#include <iostream>
#include <string_view>

#include <detail/get_env.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <detail/unaligned_span.hpp>
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
    namespace detail{
        // free function to prevent Static Initialization Order Fiasco
        inline bool is_default_colored() noexcept {
            static const bool colored =
                detail::is_env_set("FORCE_COLOR") || !detail::is_env_set("NO_COLOR");
            return colored;
        }
    }
    
    template<class L>
    class out_console_mixin : public L
    {
    public:
        out_console_mixin() noexcept = default;

        out_console_mixin(const out_console_mixin&) = delete;
        out_console_mixin& operator=(const out_console_mixin&) = delete;
        out_console_mixin(out_console_mixin&&) = delete;
        out_console_mixin& operator=(out_console_mixin&&) = delete;

        ~out_console_mixin() noexcept {
            flush();
        }

        error_code set_stream(std::ostream* stream_ptr) noexcept {
            // if already initialized
            if (stream_ptr_ != nullptr) {
                return error_code::double_init;
            }
            // only global, standard cout cerr clog is usable
            if (stream_ptr != &std::cout
                && stream_ptr != &std::cerr
                && stream_ptr != &std::clog)
            {
                return error_code::input_bad;
            }
            // if std::cout/cerr/clog was nullptr or bad
            if (stream_ptr == nullptr || !stream_ptr->good()) {
                return error_code::stream_bad;
            }
            //this can not throw, stream was good() (no error state)
            stream_ptr->exceptions(std::ios_base::iostate(0));
            stream_ptr_ = stream_ptr;
            return error_code::none;
        }

        void write_message(byte_span_const msg) noexcept {
            FSTLOG_ASSERT(stream_ptr_ != nullptr);
            FSTLOG_ASSERT(msg.data_bytes() != nullptr || msg.size_bytes() == 0);
            const std::size_t bytes = msg.size_bytes();
            if (bytes == 0) return;
            const char* const data = safe_reinterpret_cast<const char*>(msg.data_bytes());

            if (!colored_) {
                //can not throw (stream_ptr_->exceptions(0))
                stream_ptr_->write(data, bytes);
                return;
            }
            
            std::size_t last_pos = 0;
            std::size_t in_pos = 0;
            while (in_pos < bytes) {
                if (data[in_pos] < 'D' || data[in_pos] > 'W') {
                    ++in_pos;
                    continue;
                }
                
                bool matched = false;

                for (std::size_t i = 0; i < severities_.size(); ++i) {
                    const auto sev = severities_[i];
                    const std::size_t sev_len = sev.size();

                    if (bytes - in_pos >= sev_len && std::memcmp(data + in_pos, sev.data(), sev_len) == 0) {
                        if (in_pos > last_pos) {
                            stream_ptr_->write(data + last_pos, in_pos - last_pos);
                        }

                        stream_ptr_->write(colored_severities_[i].data(), colored_severities_[i].size());

                        in_pos += sev_len;
                        last_pos = in_pos;
                        matched = true;
                        break;
                    }
                }

                if (!matched) {
                    ++in_pos;
                }
            }

            if (last_pos < bytes) {
                stream_ptr_->write(data + last_pos, bytes - last_pos);
            }
        }

        void flush() noexcept {
            if (stream_ptr_ != nullptr) {
                //can not throw (stream_.exceptions(0))
                stream_ptr_->flush();
            }
        }

    private:
        std::ostream* stream_ptr_{ nullptr };
        const bool colored_{ detail::is_default_colored() };
        
        static constexpr std::array<std::string_view, 6> severities_{
            "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
        };

        static constexpr std::array<std::string_view, 6> colored_severities_{
            "\033[38;5;30;1mTRACE\033[0m",
            "\033[38;5;37;1mDEBUG\033[0m",
            "\033[38;5;34;1mINFO\033[0m",
            "\033[38;5;208;1mWARN\033[0m",
            "\033[38;5;160;1mERROR\033[0m",
            "\033[38;5;196;1mFATAL\033[0m"
        };
        static_assert(colored_severities_.size() == severities_.size(), "Unmatched severity strings!");
    };
}
