//Copyright © 2026, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <climits>
#include <cstddef>
#include <cstring>
#include <limits>
#include <utility>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h> // moving write_to_os() to out_file_win.cpp and declaring using handle_type = void*; avoids exposing <windows.h> in the header.
#else
#include <cerrno>
#include <unistd.h>
#endif

#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/memory_resource.hpp>

namespace fstlog {
    // not thread-safe
    class out_file {
    public:
#ifdef _WIN32
        typedef HANDLE handle_type;
#else
        typedef int handle_type;
#endif 
        out_file() noexcept = default;
        explicit out_file(memory_resource* resource) noexcept;
        out_file(const out_file&) = delete;
        out_file& operator=(const out_file&) = delete;
        out_file(out_file&& other) noexcept;
        // assignment and destructor discards objects,
        // call flush(); sync(); close(); and error(); in this order
        // to be able to catch more error with error()
        out_file& operator=(out_file&& other) noexcept;
        ~out_file() noexcept;

        // opens a regular file for output
        // paths are not sanitized, the caller must ensure 
        // that the path is valid and the parent directory exists
        // buffer_size == 0 selects unbuffered mode
        // buffer_size > 0 will be clamped [4096, INT_MAX] without raising an error,
        // if necessary call buffer_size() to check the clamped size
        // if it returns error_code::interrupted the caller can decide to retry (POSIX only)
        [[nodiscard]] error_code open(
            const char* file_path,
            bool truncate,
            std::size_t buffer_size) noexcept;
        
        // if succeeds reopens the log file (external log rotation, log deletion)
        // if fails the log file is not changed
        // if it returns error_code::interrupted the caller can decide to retry (POSIX only)
        // On Windows with legacy (non-POSIX) delete semantics, rolling with delete + re_open() will fail
        // always use rename/move + re_open()
        // deferred write errors can surface on POSIX, call error() to check
        // flush() is not called in this function, data in the buffer 
        // will be written to the new file after a successful re_open()
        // unless you call flush() before re_open() to flush all data to the
        // old file (but be aware, if the file was deleted this data will be lost)
        [[nodiscard]] error_code re_open(const char* file_path) noexcept;
        
        // calls flush() and closes the file handle
        // but does not call sync()
        // call flush(); sync(); close(); if you want that
        // I/O errors are reported through error()
        void close() noexcept;
        
        // sync() requests synchronization of the open file. 
        // whether the storage stack honours it is outside the program's control.
        // It does not synchronize its parent directory (POSIX), 
        // so it does not request crash durability of a filename created by open() or re_open().
        // On windows file name durability is not an issue. 
        // It does not call flush().
        // 
        // returns 
        // error_code::none on success
        // error_code::obj_bad if out_file was not opened, 
        // error_code::interrupted if the operation was interrupted by a signal (POSIX only)
        // error_code::not_supp if sync() is not supported by the device
        // error_code::extern_err on an error
        // additionally sets error_state_ on evidence of data loss 
        // on error, data since the last successful sync may be lost, 
        // never retry sync() hoping it will fix the data loss
        // a successful sync() does not fix a previous failure ("fsyncgate")
        //
        // sync() does not loop on EINTR internally, intentionally
        // and you should not do looping either on it's error_code::interrupted return
        // it is better to re-schedule the next sync with a shorter interval or try a few times
        // 
        // sync(), blocks on physical hardware or network storage (NFS, SMB, FUSE) and can stall 
        // for seconds or minutes if a disk is failing or a network mount goes unresponsive.
        //
        // The primary reason an application installs a signal handler (SIGINT, SIGTERM, SIGALRM) 
        // without SA_RESTART is to break out of stalled blocking syscalls so
        // the program can time out or shut down cleanly. 
        //
        // If sync() would implement a loop on EINTR and be used inside close() / ~out_file(), 
        // a user pressing Ctrl+C (or a watchdog timer firing) cannot interrupt a wedged sync
        // during stack unwinding, forcing a SIGKILL (sync() is not used in close() or ~out_file())
        // 
        // If sync() is interrupted in a while (sync() == error_code::interrupted) loop
        // under a high-frequency timer signal, it can livelock or cause severe I/O amplification
        [[nodiscard]] error_code sync() noexcept;
        
        // flushes its own buffer to the OS
        // I/O errors are reported through error()
        void flush() noexcept {
            if (handle_ == bad_handle_) return;
            const bool success = flush_to_os();
            error_state_ = error_state_ || !success;
        }
        
        // writes data to the file on a best-effort
        // if it detects write errors, sets error_state_ = true (call error())
        // Calling on an uninitialized or closed file is undefined behavior 
        // Calling with data == nullptr and byte_size != 0 is undefined behavior
        void write(const char* data, std::size_t byte_size) noexcept {
            FSTLOG_ASSERT(handle_ != bad_handle_);
            FSTLOG_ASSERT(byte_size == 0 || data != nullptr);
            if (byte_size == 0) return;
            
            // fast path: if the data fits in the buffer, copy it to the buffer
            if (byte_size <= buffer_size_ - buffer_pos_) {
                std::memcpy(buffer_ + buffer_pos_, data, byte_size);
                buffer_pos_ += byte_size;
                return;
            }
            // slow path with flush
            bool success = flush_to_os();
            FSTLOG_ASSERT(buffer_pos_ == 0);
            error_state_ = error_state_ || !success;
            if (byte_size <= buffer_size_) {
                std::memcpy(buffer_, data, byte_size);
                buffer_pos_ = byte_size;
                return;
            }

            // if the data is larger than the buffer, we can leave out the buffer and write directly to the OS
            // in practice this will happen only for very large log messages
            success = write_to_os(data, byte_size);
            error_state_ = error_state_ || !success;
        }

        [[nodiscard]] std::size_t buffer_size() const noexcept {
            return buffer_size_;
        }
        
        [[nodiscard]] bool error() const noexcept {
            return error_state_;
        }
        
        void clear_error() noexcept {
            error_state_ = false;
        }

    private:

        // creates the file handle for a regular file
        // returns error_code::none and the handle if succeeds
        // error_code and bad_handle_ if fails
        [[nodiscard]] std::pair<error_code, handle_type> create_file_handle(
            const char* file_path, bool truncate) noexcept;

        // closes the opened file regardless of return value
        // returns false if a detects a deferred write error (POSIX only)
        // true otherwise
        [[nodiscard]] bool close_file_handle() noexcept;

        [[nodiscard]] error_code allocate_buffer(std::size_t size) noexcept;
        void deallocate_buffer() noexcept;
        void steal_from(out_file& other) noexcept;

        // writes data to the file (OS buffer), retries on EINTR or partial writes
        // returns true on success, false on any error
        [[nodiscard]] bool write_to_os(const char* data, std::size_t byte_size) noexcept {
            FSTLOG_ASSERT(handle_ != bad_handle_);
            FSTLOG_ASSERT(byte_size == 0 || data != nullptr);
            while (byte_size > 0) {
#ifdef _WIN32
                const DWORD bytes_to_write = byte_size > max_chunk_bytes_ ? 
                    static_cast<DWORD>(max_chunk_bytes_) : static_cast<DWORD>(byte_size);
                DWORD bytes_written = 0;
                const auto success = WriteFile(handle_, data, bytes_to_write, &bytes_written, nullptr);
                if (success == FALSE || bytes_written == 0) return false; // failure + no progress
                byte_size -= bytes_written;
                data += bytes_written;
#else
                const std::size_t bytes_to_write = byte_size > max_chunk_bytes_ ? max_chunk_bytes_ : byte_size;
                const auto result = ::write(handle_, data, bytes_to_write);
                if(result > 0){
                    byte_size -= static_cast<std::size_t>(result);
                    data += result;
                }
                // retry on EINTR, break on all else (0 bytes written)
                else if (result != -1 || errno != EINTR) {
                    return false;
                }        
#endif
            }
            return true;
        }

        // flushes it's own buffer to the OS
        // returns false on error
        [[nodiscard]] bool flush_to_os() noexcept {
            const bool success = write_to_os(buffer_, buffer_pos_);
            // for performance reasons we always reset the buffer (less sys call on permanent error, e.g. disk full)
            buffer_pos_ = 0;
            return success;
        }

        // maximum/minimum buffer size, if you change these, check the API requirements!
        static constexpr std::size_t max_buffer_size_ = INT_MAX;
        static constexpr std::size_t min_buffer_size_ = 4096;
        // limit writes chunks to a reasonable size to prevent fail with no system resource on transfers
        static constexpr std::size_t max_chunk_bytes_ = 64 * 1024 * 1024;
        
        handle_type handle_{ bad_handle_ };
        char* buffer_{ nullptr };
        std::size_t buffer_size_{ 0 };
        std::size_t buffer_pos_{ 0 };
        memory_resource* memory_resource_{ nullptr };
        bool error_state_{ false };

#ifdef _WIN32
        static constexpr handle_type bad_handle_{ nullptr };
        static_assert(max_chunk_bytes_ <= (std::numeric_limits<DWORD>::max)(),"max_chunk_bytes too big!");

#else
        static constexpr handle_type bad_handle_{ -1 };
        static_assert(max_chunk_bytes_ <= (std::numeric_limits<ssize_t>::max)() &&
            max_chunk_bytes_ <= INT_MAX, "max_chunk_bytes too big!");
#endif
        static_assert(min_buffer_size_ <= max_buffer_size_, "Minimum buffer size must not exceed maximum buffer size");
    };
}
