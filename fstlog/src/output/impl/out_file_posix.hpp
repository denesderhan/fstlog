//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>

#if (defined(_WIN32))
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <io.h>
#include <detail/utf_conv.hpp>
#include <detail/unaligned_span.hpp>
#else
#include <unistd.h>
#endif

#include <detail/nothrow_allocate.hpp>
#include <detail/constants_src.hpp>
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/memory_resource.hpp>

namespace fstlog {
    // non thread-safe
    class out_file_posix {
    public:
        out_file_posix() noexcept = default;
        
        explicit out_file_posix(memory_resource* resource) noexcept
            : memory_resource_{ resource } 
        {
            // We prefer an error on a nullptr resource (allocate_buffer will fail) 
            // instead of an unintended use of a default memory resource.
            FSTLOG_ASSERT(resource != nullptr);
        }

        out_file_posix(const out_file_posix&) = delete;
        out_file_posix& operator=(const out_file_posix&) = delete;
        
        out_file_posix(out_file_posix&& other) noexcept {
            steal_from(other);
        }

        out_file_posix& operator=(out_file_posix&& other) noexcept {
            if (this == &other) return *this;
            close();
            deallocate_buffer();
            steal_from(other);
            return *this;
        }

        ~out_file_posix() noexcept {
            close();
            deallocate_buffer();
        }

        // buffer_size == 0 selects unbuffered mode
        // buffer_size > 0 will be clamped (4096,INT_MAX) without raising an error,
        // if necessary call buffer_size() to check the clamped size
        [[nodiscard]] error_code open(
            const char* file_path,
            bool truncate,
            std::size_t buffer_size) noexcept
        {
            if (handle_ != nullptr)  return error_code::double_init;
            if (file_path == nullptr || file_path[0] == '\0') return error_code::path_bad;
            
            auto ec = allocate_buffer(buffer_size);
            if (ec != error_code::none) {
                return ec;
            }

            FSTLOG_ASSERT(
                (buffer_size_ == 0 && buffer_ == nullptr)
                || (buffer_size_ >= min_buffer_size_ && buffer_ != nullptr));
                        
            
            ec = create_file_handle(handle_, file_path, truncate);
            if (ec != error_code::none) {
                return ec;
                // deallocation of buffer is handled in destructor 
                // allowing the user to call open() multiple times with different parameters (after close() if previously opened successfully)
                // without fragmenting memory (the buffer is reused if the size is the same)
            }

            FSTLOG_ASSERT(handle_ != nullptr);
            const auto buffer_mode = buffer_size_ == 0 ? _IONBF : _IOFBF;
            if (std::setvbuf(handle_, buffer_, buffer_mode, buffer_size_) != 0) {
                close(); // close the file
                // we leave the buffer allocated, see comment above
                return error_code::extern_err;
            }
            return error_code::none;
        }

        void close() noexcept {
            if (handle_ != nullptr) {
                std::fclose(handle_);
                handle_ = nullptr;
            }
        }

        void flush() noexcept {
            if (handle_ == nullptr) return;
            (void) std::fflush(handle_);
        }

        // calling on an uninitialized or closed file is undefined behavior 
        // calling with data == nullptr and byte_size != 0 is undefined behavior
        void write(const char* data, std::size_t byte_size) noexcept {
            FSTLOG_ASSERT(handle_ != nullptr && (data != nullptr || byte_size == 0));
            (void) std::fwrite(data, 1, byte_size, handle_);
        }

        [[nodiscard]] std::size_t buffer_size() const noexcept {
            return buffer_size_;
        }

        [[nodiscard]] bool is_open() const noexcept {
            return handle_ != nullptr;
        }

    private:
        [[nodiscard]] error_code allocate_buffer(std::size_t size) noexcept {
            // unbuffered mode is indicated by buffer_size == 0
            if (size == 0) {
                deallocate_buffer();
                return error_code::none;
            }

            if (size < min_buffer_size_) size = min_buffer_size_;
            else if (size > max_buffer_size_) size = max_buffer_size_;

            if (size == buffer_size_) {
                FSTLOG_ASSERT(buffer_ != nullptr);
                return error_code::none; // already allocated with the same size
            }

            if (memory_resource_ == nullptr) {
                return error_code::mem_res_null;
            }

            // deallocating before new allocation to avoid memory exhaustion on limited memory systems
            // Since open() then fails and the caller has to react anyway, that's acceptable
            deallocate_buffer();
            
            buffer_ = static_cast<char*>(aligned_nothrow_allocate(
                memory_resource_,
                size,
                constants::cache_ls_nosharing));
            if (buffer_ == nullptr) {
                return error_code::alloc_fail;
            }
            buffer_size_ = size;
            return error_code::none;
        }

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

        [[nodiscard]] fstlog::error_code create_file_handle(
            std::FILE* &handle, const char* file_path, bool truncate) noexcept 
        {
#if (defined(_WIN32))
            unaligned_span<const unsigned char> input(
                safe_reinterpret_cast<const unsigned char*>(file_path),
                std::strlen(file_path));
            wchar_t path[max_path_length_]{ 0 };
            unaligned_span output(path, max_path_length_ - 1); // Leave space for null terminator
            const auto result = detail::utf::safe_utf8_to_utf16(input, output);
            if (result.ec != error_code::none) {
                if (result.ec == error_code::buff_full) return error_code::str_long;
                else return error_code::utf_invalid;
            }

            // FILE_SHARE_DELETE allows another handle to rename or delete this file (e.g., logrotate rename)
            // FILE_SHARE_WRITE allows another handle to write to this file (e.g., copy/truncate or manual clear)
            // FILE_SHARE_READ allows read
            HANDLE os_handle = ::CreateFileW(
                path,
                FILE_APPEND_DATA | FILE_WRITE_DATA | SYNCHRONIZE,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                nullptr,
                (truncate ? CREATE_ALWAYS : OPEN_ALWAYS),
                FILE_ATTRIBUTE_NORMAL,
                nullptr);

            if (os_handle == INVALID_HANDLE_VALUE) {
                return error_code::file_create_fail;
            }

            int fd = _open_osfhandle( reinterpret_cast<intptr_t>(os_handle), _O_APPEND );
            if (fd == -1) {
                ::CloseHandle(os_handle);
                return error_code::file_create_fail;
            }
            (void) _setmode(fd, _O_BINARY);
            
            std::FILE* file = _fdopen(fd, "ab");
            if (!file) {
                _close(fd); // also closes the underlying HANDLE
                return error_code::file_create_fail;
            }
            handle = file;
#else
            int fd;
            do {
                // O_CLOEXEC prevents resource leaks to child processes, which is important for security and resource management.
                // O_APPEND ensures that writes are correct when log file gets emptied (logrotate copy/truncate, echo >> app.log).
                fd = ::open(file_path,
                    O_WRONLY | O_CREAT | O_CLOEXEC | O_APPEND | (truncate ? O_TRUNC : 0),
                    0640);
            // open() can fail with EINTR if it blocks and a signal arrives whose handler was installed without SA_RESTART 
            // (common in event loops that want interruptible syscalls, some profilers, etc.)
            } while (fd < 0 && errno == EINTR); 
            
            if (fd < 0) {
                return error_code::file_create_fail;
            }
#ifndef O_CLOEXEC
            fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif
            // Wrapping into a standard C FILE* stream
            std::FILE* file = ::fdopen(fd, "ab");
            if (!file) {
                ::close(fd);
                return error_code::file_create_fail;
            }
            handle = file;
#endif
            return error_code::none;
        }

        void steal_from(out_file_posix& other) noexcept {
            if (this == &other) return;
            FSTLOG_ASSERT(handle_ == nullptr 
                && buffer_ == nullptr && buffer_size_ == 0);
            handle_ = other.handle_;
            buffer_ = other.buffer_;
            buffer_size_ = other.buffer_size_;
            memory_resource_ = other.memory_resource_;
            other.handle_ = nullptr;
            other.buffer_ = nullptr;
            other.buffer_size_ = 0;
            // we do not reset the memory_resource_ of the other object 
            // to prevent errors using it after move (it is not a dangling pointer, but the user should not use it after move)
        }

        // for memory usage optimization, we limit the maximum path length to 512 characters on Windows (1024 byte buffer)
        // (we don't need an intermediate buffer on POSIX systems)
        static constexpr std::size_t max_path_length_ = 512;
        // maximum/minimum buffer size, if you change these, check the API requirements!
        static constexpr std::size_t max_buffer_size_ = INT_MAX;
        static constexpr std::size_t min_buffer_size_ = 4096;

        std::FILE* handle_{ nullptr };
        char* buffer_{ nullptr };
        std::size_t buffer_size_{ 0 };
        memory_resource* memory_resource_{ nullptr };
        
        static_assert(min_buffer_size_ <= max_buffer_size_, "Minimum buffer size must not exceed maximum buffer size");
    };
}
