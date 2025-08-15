//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#ifdef FSTLOG_DEBUG
#include <cstring>
#endif
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/ut_cast.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
    class error {
    public:

        error() noexcept = default;
        error(const char* file, int line, error_code err) noexcept
            :file_{ file }, line_{ line }, ec_{ err } {}
        const char* message() const noexcept {
            return fstlog::error_message(ec_);
        }
        
        const char* file() const noexcept {
            if (file_ != nullptr) return file_;
            else return "?";
        }
        int line() const noexcept {
            return line_;
        }
        error_code code() const noexcept {
            return ec_;
        }

        unsigned char* write_to(unsigned char* begin, const unsigned char* end) const noexcept {
            if (begin == nullptr || end == nullptr || begin >= end) return begin;
            int err_num = ut_cast(code());
            std::size_t max_len = end - begin;
            if (max_len < 7) return begin;
            //  7 <= buff < 20
            if (max_len < 20) {
                begin = write_cstring("Err:", begin);
                begin = write_num(err_num, begin);
            }
            //  20 <= buff < 100
            else if(max_len < 100) {
                begin = write_cstring("... fstlog error:", begin);
                begin = write_num(err_num, begin);
            }
            //  100 <= buff
            else {
                const char* text = "... Message truncated, ";
                FSTLOG_ASSERT(message() != nullptr);
                FSTLOG_ASSERT(std::strlen(text) + std::strlen(message()) <= 100 );
                begin = write_cstring(text, begin);
                begin = write_cstring(message(), begin);
#ifdef FSTLOG_DEBUG
                max_len = end - begin;
                if (max_len >= 1 + std::strlen(file()) + 1 + 3) {
                    *begin++ = ' ';
                    begin = write_cstring(file(), begin);
                    *begin++ = ':';
                    begin = write_num(line(), begin);
                }
#endif
            }
            return begin;
        }

    private:
        unsigned char* write_cstring(const char* str, unsigned char* pos) const noexcept {
            while (*str != 0) *pos++ = static_cast<unsigned char>(*str++);
            return pos;
        }

        unsigned char* write_num(int num, unsigned char*& begin) const noexcept {
            if (num < 0 || num > 999) {
                *begin++ = '?';
                return begin;
            }
            *(begin + 2) = static_cast<unsigned char>('0' + num % 10);
            num /= 10;
            *(begin + 1) = static_cast<unsigned char>('0' + num % 10);
            num /= 10;
            *begin = static_cast<unsigned char>('0' + num % 10);
            begin += 3;
            return begin;
        }

        const char* file_{ nullptr };
        int line_{ -1 };
        error_code ec_{ error_code::none};
    };
}
