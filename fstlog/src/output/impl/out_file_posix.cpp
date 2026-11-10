//Copyright © 2026, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#ifndef _WIN32

#include <output/impl/out_file.hpp>

#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#ifndef O_CLOEXEC
# error "O_CLOEXEC (atomic close-on-exec) is required to prevent leaking file descriptors. This platform is too old (pre 2.6.23) or incomplete."
#endif

#include <fstlog/detail/error_code.hpp>

namespace fstlog {
    namespace {
        inline int safe_close(int fd) noexcept {
#if defined(POSIX_CLOSE_RESTART) // close() EINTR behaviour is defined
#   if POSIX_CLOSE_RESTART == 0 // handle will be closed (no leak)
            return ::close(fd);
#   else // safe to retry after EINTR 
            int result;
            do {
                result = ::close(fd);
            } while (result == -1 && errno == EINTR);
            return result;
#   endif 
#else
            // implementation is unknown (linux always closes the handle)
            // on a best effort we do not retry on EINTR
            // avoid a possible close of a reused file descriptor over avoid a possible leak
            return ::close(fd);
#endif
        }
    }


    [[nodiscard]] error_code out_file::sync() noexcept {
        if (handle_ == bad_handle_) return error_code::obj_bad;
        int result{ -1 };
        int err{ 0 };
#if defined(F_FULLFSYNC)
        result = ::fcntl(handle_, F_FULLFSYNC);
        err = errno;
        // fallback to weaker fsync() if F_FULLFSYNC is not supported
        // but fail on any other error
        if (result == -1 && (
            err == ENOTTY || err == ENOTSUP 
            || err == EOPNOTSUPP || err == EINVAL))
        {
            result = ::fsync(handle_);
            err = errno;
        }
#elif defined(_POSIX_SYNCHRONIZED_IO) && (_POSIX_SYNCHRONIZED_IO > 0)
        result = ::fdatasync(handle_);
        err = errno;
#else // F_FULLFSYNC/fdatasync() absent 
        result = ::fsync(handle_);
        err = errno;
#endif       
        // mac's fcntl only guarantees to return -1 on error and something other than -1 on success
        if (result != -1) return error_code::none; 
        
        // detect I/O errors
        if (err == EIO || err == ENOSPC || err == EDQUOT) {
            error_state_ = true;
        }
        if (err == EINTR) return error_code::interrupted;
        // sync is not supported 
        if (err == EINVAL || err == ENOTSUP || err == EOPNOTSUPP) 
        {
            return error_code::not_supp;
        }
        
        // sync failed
        return error_code::extern_err;
    }
    
    // Returns true unless there is positive evidence that buffered
    // writes were not committed (we only care about durability here,
    // not about procedural/logic errors from close() itself).
    // true == there can be write failures, we can't be sure
    // false == there is definitely a write failure
    [[nodiscard]] bool out_file::close_file_handle() noexcept {
        if (handle_ == bad_handle_) return true;
        const int result = safe_close(handle_);
        const int err = errno;
        handle_ = bad_handle_;
        if (result == 0) return true; // no close error was reported, no evidence of non-durability

        // Only treat errno values that specifically indicate a failed
        // write-back / storage allocation as "evidence of no durability".
        // Everything else (EINTR left unretried, EBADF from a logic bug,
        // EINVAL, etc.) is a procedural error, not evidence the data
        // didn't make it to disk, so we deliberately do not fail on it.
        if (err == EIO || err == ENOSPC || err == EDQUOT) {
            return false; // evidence of no durability
        }
        
        return true;  // no evidence of non-durability
    }

    [[nodiscard]] std::pair<error_code, out_file::handle_type> out_file::create_file_handle(
        const char* file_path, bool truncate) noexcept
    {
        constexpr auto error_fail = std::make_pair(error_code::file_create_fail, bad_handle_);
        constexpr auto error_eintr = std::make_pair(error_code::interrupted, bad_handle_);

        if (file_path == nullptr || file_path[0] == '\0') return error_fail;
        const int fd = ::open(file_path,
            O_WRONLY | O_CREAT | O_APPEND // open for append, write
            | O_NOCTTY // Don't make this file the process's controlling terminal (prevents terminal hijacking).
            | O_CLOEXEC // Close fd on exec() — prevents fd leakage to child processes (security).
            | O_NONBLOCK, // Prevent blocking open() when opening a pipe (accidental or attack)
            0640);
        if (fd == -1) {
            if(errno == EINTR) return error_eintr;
            return error_fail;
        }
        
        // Checks if the opened item is not a Regular File.
        struct stat st;
        if (::fstat(fd, &st) != 0) {
            const int err = errno;
            safe_close(fd);
            if (err == EINTR) return error_eintr;
            return error_fail;
        }
        if (!S_ISREG(st.st_mode)) {
            safe_close(fd);
            return error_fail;
        }

        // clear O_NONBLOCK (we want blocking writes)
        int flags = ::fcntl(fd, F_GETFL);
        if (flags == -1 || ::fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) == -1) {
            const int err = errno;
            safe_close(fd);
            if (err == EINTR) return error_eintr;
            return error_fail;
        }

        // cautious truncate after all checks/ops succeeded
        if (truncate) {
            if (::ftruncate(fd, 0) != 0) {
                const int err = errno;
                safe_close(fd);
                if (err == EINTR) return error_eintr;
                return error_fail;
            }
        }

        return { error_code::none, fd };
    }
}
#endif
