//Copyright © 2026, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#ifdef _WIN32
#include <output/impl/out_file.hpp>

#include <windows.h>
#include <winioctl.h>
#include <cstddef>
#include <cstring>
#include <climits>
#include <utility>

#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/memory_resource.hpp>
#include <detail/dyn_array.hpp>

namespace fstlog {
    
    namespace {
        std::pair<error_code, dyn_array<wchar_t>> make_utf16_path(const char* file_path) noexcept
        {
            if (file_path == nullptr || file_path[0] == '\0') {
                return { error_code::path_bad, dyn_array<wchar_t>() };
            }
            const std::size_t file_path_bytes = std::strlen(file_path);
            if(file_path_bytes > INT_MAX) {
                return { error_code::str_long, dyn_array<wchar_t>() };
            }

            // counting the UTF-16 code units needed excluding null terminator
            const int buff_size = MultiByteToWideChar(
                CP_UTF8,
                MB_ERR_INVALID_CHARS,
                file_path,
                static_cast<int>(file_path_bytes),
                nullptr,
                0);

            if (buff_size == 0) {
                if (GetLastError() == ERROR_NO_UNICODE_TRANSLATION) {
                    return { error_code::utf_invalid, dyn_array<wchar_t>() };
                }
                else {
                    return { error_code::extern_err, dyn_array<wchar_t>() };
                }
            }
                        
            // we allocate from the default resource
            // (if the custom resource's size is restricted, we don't want to fail on a path conversion)
            dyn_array<wchar_t> utf16_path(get_default_resource());
            if (!utf16_path.resize(static_cast<std::size_t>(buff_size) + 1)) {
                return { error_code::alloc_fail, dyn_array<wchar_t>() };
            }

            const auto result = MultiByteToWideChar(
                CP_UTF8,
                MB_ERR_INVALID_CHARS,
                file_path,
                static_cast<int>(file_path_bytes),
                utf16_path.data(),
                buff_size);

            if (result != buff_size) {
                return { error_code::extern_err, dyn_array<wchar_t>() };
            }
            utf16_path[buff_size] = L'\0';
            return { error_code::none, std::move(utf16_path) };
        }

        [[nodiscard]] bool truncate_file(HANDLE handle, DWORD share_mode) noexcept {
            const auto trunc_handle = ReOpenFile(handle, GENERIC_WRITE, share_mode, 0);
            if (trunc_handle == INVALID_HANDLE_VALUE) return false;
            auto success = SetFilePointer(trunc_handle, 0, 0 , FILE_BEGIN) != INVALID_SET_FILE_POINTER;
            if (success) {
                success = SetEndOfFile(trunc_handle);
            }
            CloseHandle(trunc_handle);
            return success;
        }

        [[nodiscard]] bool banned_device(HANDLE handle) noexcept {
            const auto type = GetFileType(handle);
            if (type != FILE_TYPE_DISK) return true;
            
            // detect raw devices
            DISK_GEOMETRY dg = {};
            DWORD bytesReturned = 0;
            // querying disk geometry works only for raw devices (may fail on exotic devices?)
            BOOL result = DeviceIoControl(
                handle,
                IOCTL_DISK_GET_DRIVE_GEOMETRY,
                nullptr,
                0,
                &dg,
                sizeof(dg),
                &bytesReturned,
                nullptr
            );
            return result == TRUE;
        }
    }
    
    [[nodiscard]] error_code out_file::sync() noexcept {
        if (handle_ == bad_handle_) return error_code::obj_bad;
        if(FlushFileBuffers(handle_)) return error_code::none;
        const auto error = GetLastError();
        // unsupported
        if (error == ERROR_NOT_SUPPORTED
            || error == ERROR_INVALID_FUNCTION
            || error == ERROR_CALL_NOT_IMPLEMENTED)
        {
            return error_code::not_supp;
        }
        
        error_state_ = true;
        return error_code::extern_err;
    }

    // close on windows does not report I/O errors
    // always returns true
    [[nodiscard]] bool out_file::close_file_handle() noexcept {
        if (handle_ == bad_handle_) return true;
        [[maybe_unused]] const auto success = CloseHandle(handle_);
        FSTLOG_ASSERT(success != FALSE);
        handle_ = bad_handle_;
        return true;
    }

    [[nodiscard]] std::pair<error_code, out_file::handle_type> out_file::create_file_handle(
        const char* file_path, bool truncate) noexcept
    {
        if (file_path == nullptr || file_path[0] == '\0') {
            return { error_code::path_bad, bad_handle_ };
        }
        const auto [ec, utf16_path] = make_utf16_path(file_path);
        if (ec != error_code::none) {
            return { ec, bad_handle_ };
        }
        
        // FlushFileBuffers works with a handle that has only FILE_APPEND_DATA.
        // Although MSDN mentions GENERIC_WRITE, FlushFileBuffers delegates to
        // NtFlushBuffersFile, where the I/O manager checks that the handle's granted
        // access includes either FILE_WRITE_DATA or FILE_APPEND_DATA.
        auto access_mode = GENERIC_READ | FILE_APPEND_DATA | SYNCHRONIZE;
        // FILE_SHARE_DELETE allows another handle to rename or delete this file (e.g., logrotate rename)
        // FILE_SHARE_WRITE allows another handle to write to this file (e.g., copy/truncate or manual clear)
        // FILE_SHARE_READ allows read
        constexpr auto share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
        constexpr auto open_mode = OPEN_ALWAYS;
        constexpr auto file_attrib = FILE_ATTRIBUTE_NORMAL;

        // CreateFileW() will fail for directories (no FILE_FLAG_BACKUP_SEMANTICS)
        // root is a directory (\\.\C:\)
        
        // Try to open with read access first to have better performance on network drives (e.g., SMB)
        handle_type os_handle = ::CreateFileW(
            utf16_path.data(),
            access_mode,
            share_mode,
            nullptr,
            open_mode,
            file_attrib,
            nullptr);

        // Retry without read access (we intentionally check only the handle to not miss any open errors) 
        // (e.g., a log file created by another process with write-only access)
        if (os_handle == INVALID_HANDLE_VALUE) {
            access_mode = FILE_APPEND_DATA | SYNCHRONIZE;
            os_handle = ::CreateFileW(
                utf16_path.data(),
                access_mode,
                share_mode,
                nullptr,
                open_mode,
                file_attrib,
                nullptr);
        }

        if (os_handle == INVALID_HANDLE_VALUE) {
            return { error_code::file_create_fail, bad_handle_ };
        }
                    
        if (banned_device(os_handle)) {
            CloseHandle(os_handle);
            return { error_code::file_create_fail, bad_handle_ };
        }

        // cautious truncating after all checks/ops succeeded
        if (truncate) {
            if (!truncate_file(os_handle, share_mode)) {
                CloseHandle(os_handle);
                return { error_code::file_create_fail, bad_handle_ };
            }
        }
        return { error_code::none, os_handle };
    }
}
#endif
