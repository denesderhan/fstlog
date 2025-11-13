//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstdlib>
#include <iostream>

#include <detail/safe_reinterpret_cast.hpp>
#include <detail/unaligned_span.hpp>
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/small_string.hpp>

namespace fstlog {
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
            FSTLOG_ASSERT(msg.data_bytes() != nullptr);
            const char* data = safe_reinterpret_cast<const char*>(msg.data_bytes());
            std::size_t bytes = msg.size_bytes();
            //can not throw (stream_ptr_->exceptions(0))
            if (!colored_) {
                stream_ptr_->write(data, bytes);
            }
            else {
                std::size_t in_pos{ 0 };
                std::size_t last_copy_end{ 0 };
                int found_index = -1;
                std::array<int, 6> comp_pos{ 0 };
                while (in_pos < bytes) {
                    for (int i = 0; i < comp_pos.size(); i++) {
                        const auto& severity = severities_[i];
                        auto& severity_pos = comp_pos[i];
                        if (data[in_pos] == severity[severity_pos]) {
                            severity_pos++;
                            // if we found the full severity string
                            if (severity_pos == severity.size()) {
                                found_index = i;
                                break;
                            }
                        }
                        else {
                            // if we find a non matching character we have to restart from severity string beginning
                            severity_pos = 0;
                        }
                    }
                    in_pos++;
                    if (found_index != -1) {
                        // write chars up to the severity string
                        auto in_severity_pos = in_pos
                            - severities_[found_index].size();
                        auto write_len = in_severity_pos - last_copy_end;
                        stream_ptr_->write(data + last_copy_end, write_len);
                        // write the colored severity string
                        stream_ptr_->write(
                            colored_severities_[found_index].data(),
                            colored_severities_[found_index].size());
                        // update input pos to after the severity string 
                        last_copy_end = in_pos;
                        // reset severity string compare positions
                        comp_pos.fill(0);
                        found_index = -1;
                    }
                }
                if (last_copy_end < bytes) {
                    stream_ptr_->write(data + last_copy_end, bytes - last_copy_end);
                }
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
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996) // safe to disable, we never dereference the returned pointer
#endif
        bool colored_{ std::getenv("NO_COLOR") == nullptr };
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
        static constexpr std::array<small_string<8>, 6> severities_{
            small_string<8>{"TRACE"},
            small_string<8>{"DEBUG"},
            small_string<8>{"INFO"},
            small_string<8>{"WARN"},
            small_string<8>{"ERROR"},
            small_string<8>{"FATAL"}
        };

        static constexpr std::array<small_string<24>, 6> colored_severities_{
            small_string<24>{"\033[38;5;30;1mTRACE\033[0m"},
            small_string<24>{"\033[38;5;37;1mDEBUG\033[0m"},
            small_string<24>{"\033[38;5;34;1mINFO\033[0m"},
            small_string<24>{"\033[38;5;208;1mWARN\033[0m"},
            small_string<24>{"\033[38;5;160;1mERROR\033[0m"},
            small_string<24>{"\033[38;5;196;1mFATAL\033[0m"}
        };
    };
}
