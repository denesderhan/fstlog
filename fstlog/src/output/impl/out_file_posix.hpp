//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <cstdio>
#include <cstring>

#include <detail/nothrow_allocate.hpp>
#include <detail/constants_src.hpp>
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/memory_resource.hpp>
#include <detail/utf_conv.hpp>

namespace fstlog {
    class out_file_posix {
    public:
        out_file_posix() noexcept = default;
        
        explicit out_file_posix(memory_resource* resource) noexcept
            : memory_resource_{ resource } {
        }

        out_file_posix(const out_file_posix&) = delete;
        out_file_posix& operator=(const out_file_posix&) = delete;
        
        out_file_posix(out_file_posix&& other) noexcept {
            handle_ = other.handle_;
            buffer_ = other.buffer_;
            buffer_size_ = other.buffer_size_;
            memory_resource_ = other.memory_resource_;
            other.handle_ = nullptr;
            other.buffer_ = nullptr;
            other.buffer_size_ = 0;
            other.memory_resource_ = nullptr;
        }

        out_file_posix& operator=(out_file_posix&& other) noexcept {
            FSTLOG_ASSERT(
                (handle_ == nullptr || handle_ != other.handle_)
                && (buffer_ == nullptr || buffer_ != other.buffer_));
            close();
            deallocate_buffer();
            handle_ = other.handle_;
            buffer_ = other.buffer_;
            buffer_size_ = other.buffer_size_;
            memory_resource_ = other.memory_resource_;
            other.handle_ = nullptr;
            other.buffer_ = nullptr;
            other.buffer_size_ = 0;
            other.memory_resource_ = nullptr;
            return *this;
        }

        ~out_file_posix() noexcept {
            close();
            deallocate_buffer();
        }

        error_code open(
            const char* file_path,
            bool truncate,
            std::size_t buffer_size) noexcept
        {
            if (handle_ != nullptr) return error_code::double_init;
            if (file_path == nullptr) return error_code::path_bad;
            unaligned_span<const unsigned char> input(
                safe_reinterpret_cast<const unsigned char*>(file_path), 
                std::strlen(file_path));
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
            wchar_t path[512]{ 0 };
            unaligned_span output(path, 511); // 511 last char must be '/0'
            const auto result = detail::utf::safe_utf8_to_utf16(input, output);
            if (result.ec != error_code::none) {
                return result.ec;
            }
#else
            while (!input.empty()) {
                auto code_point = detail::utf::decode_utf8_char(input);
                if (!detail::utf::safe_utf_code_point(code_point)) {
                    return error_code::path_bad;
                }
            }
#endif
            //Win32 api: Allowable range : 2 <= size <= INT_MAX(2147483647) 
            if (buffer_size == 1) buffer_size = 2;
            else if (buffer_size > 2147483647) buffer_size = 2147483647;
            //on close buffer is not deallocated (reused if opened with same size)
            if (buffer_size != buffer_size_) {
                deallocate_buffer();
                if (buffer_size != 0) {
                    buffer_ = static_cast<char*>(aligned_nothrow_allocate(
                        memory_resource_,
                        buffer_size,
                        constants::cache_ls_nosharing));
                    if (buffer_ == nullptr) {
                        return error_code::alloc_fail;
                    }
                    buffer_size_ = buffer_size;
                }
            }
            FSTLOG_ASSERT(
                (buffer_size_ == 0 && buffer_ == nullptr)
                || (buffer_size_ != 0 && buffer_ != nullptr));
            
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
            if (truncate) {
                handle_ = _wfsopen(path, L"wb", _SH_DENYWR);
            }
            else {
                handle_ = _wfsopen(path, L"ab", _SH_DENYWR);
            }
#else
            if (truncate) {
                handle_ = std::fopen(file_path, "wb");
            }
            else { 
                handle_ = std::fopen(file_path, "ab"); 
            }
#endif
            //buffer won't be deallocated
            if (handle_ == nullptr) return error_code::path_bad;
            
            const auto buffer_mode = buffer_size_ == 0 ? _IONBF : _IOFBF;
            if (std::setvbuf(handle_, buffer_, buffer_mode, buffer_size_)) {
                close();
                return error_code::extern_err;
            }
            return error_code::none;
        }

        void try_reopen(const char* file_path) noexcept {
            FSTLOG_ASSERT(handle_ != nullptr);
            
            FILE* temp{ nullptr };
            unaligned_span<const unsigned char> input(
                safe_reinterpret_cast<const unsigned char*>(file_path),
                std::strlen(file_path));
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
            wchar_t path[512]{ 0 };
            unaligned_span output(path, 511); //511 (last char must be '/0')
            const auto result = detail::utf::safe_utf8_to_utf16(input, output);
            if (result.ec != error_code::none) {
                return; //Path was bad or too long!
            }
            _wfopen_s(&temp, path, L"ab");
#else
            while (!input.empty()) {
                auto code_point = detail::utf::decode_utf8_char(input);
                if (!detail::utf::safe_utf_code_point(code_point)) {
                    return; //Path was bad or too long!
                }
            }
            temp = std::fopen(file_path, "ab");
#endif
            if (temp == nullptr) return;

            std::fclose(handle_);

            const auto buffer_mode = buffer_size_ == 0 ? _IONBF : _IOFBF;
            //trying to set buffer_
            if (std::setvbuf(temp, buffer_, buffer_mode, buffer_size_)) {
                deallocate_buffer();
                //if fails trying to set unbuffered mode
                //if fails buffering will be system default 
                std::setvbuf(temp, nullptr, _IONBF, 0);
            }
    
            handle_ = temp;

            // The setvbuf function may be used only after the stream pointed to by stream has
            // been associated with an open file and before any other operation(other than an
            // unsuccessful call to setvbuf) is performed on the stream.
        }

        void close() noexcept {
            if (handle_ != nullptr) {
                std::fclose(handle_);
                handle_ = nullptr;
            }
        }

        void flush() noexcept {
            FSTLOG_ASSERT(handle_ != nullptr);
            std::fflush(handle_);
        }

        void write(const char* data, std::size_t byte_size) noexcept {
            FSTLOG_ASSERT(handle_ != nullptr);
            std::fwrite(data, byte_size, 1, handle_);
        }

    private:
        void deallocate_buffer() noexcept {
            if (buffer_ != nullptr) {
                aligned_nothrow_deallocate(
                    buffer_,
                    memory_resource_,
                    buffer_size_,
                    constants::cache_ls_nosharing);
                buffer_ = nullptr;
            }
            buffer_size_ = 0;
        }

        std::FILE* handle_{ nullptr };
        char* buffer_{ nullptr };
        std::size_t buffer_size_{ 0 };
        memory_resource* memory_resource_{ nullptr };
    };
}
