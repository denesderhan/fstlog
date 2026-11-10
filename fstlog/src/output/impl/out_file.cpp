//Copyright © 2026, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <output/impl/out_file.hpp>

#include <detail/nothrow_allocate.hpp>
#include <detail/constants_src.hpp>
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/memory_resource.hpp>

namespace fstlog {
    out_file::out_file(memory_resource* resource) noexcept
        : memory_resource_{ resource }
    {
        // We prefer an error on a nullptr resource (allocate_buffer will fail) 
        // instead of an unintended use of a default memory resource.
        FSTLOG_ASSERT(resource != nullptr);
    }
    
    out_file::out_file(out_file&& other) noexcept {
        steal_from(other);
    }

    out_file& out_file::operator=(out_file&& other) noexcept {
        if (this == &other) return *this;
        close();
        deallocate_buffer();
        steal_from(other);
        return *this;
    }

    out_file::~out_file() noexcept {
        close();
        deallocate_buffer();
    }

    [[nodiscard]] error_code out_file::open(
        const char* file_path,
        bool truncate,
        std::size_t buff_size) noexcept
    {
        if (handle_ != bad_handle_)  return error_code::double_init;
        if (file_path == nullptr || file_path[0] == '\0') return error_code::path_bad;

        const auto alloc_ec = allocate_buffer(buff_size);
        if (alloc_ec != error_code::none)  return alloc_ec;
        
        FSTLOG_ASSERT((buffer_size_ == 0 && buffer_ == nullptr)
            || (buffer_size_ >= min_buffer_size_ && buffer_size_ <= max_buffer_size_ 
                && buffer_ != nullptr));

        const auto [create_ec, file_handle] = create_file_handle(file_path, truncate);
        // deallocation of buffer is handled in destructor 
        // allowing the user to call open() multiple times with different parameters (after close() if previously opened successfully)
        // without fragmenting memory (the buffer is reused if the size is the same)
        if (create_ec != error_code::none) return create_ec;
            
        handle_ = file_handle;
        clear_error();
        return error_code::none;
    }

    void out_file::close() noexcept {
        if (handle_ == bad_handle_) return;
        flush();
        const bool no_write_error = close_file_handle();
        error_state_ = error_state_ || !no_write_error;
    }

    [[nodiscard]] error_code out_file::re_open(const char* file_path) noexcept {
        if (handle_ == bad_handle_) return error_code::obj_bad;
        if (file_path == nullptr || file_path[0] == '\0') return error_code::path_bad;
        // no truncation (never destroy data accidentally)
        const auto [ec, new_handle] = create_file_handle(file_path, false);
        // continue using the old handle (it is better than a crash, even if the file was deleted)
        if (ec != error_code::none) return ec;
        
        // we intentionally don't clear write errors (POSIX can report I/O error on close())
        // and don't flush the buffer
        // if the old file was deleted the data would be lost
        // the buffer will be flushed to the new file
        const bool no_write_error = close_file_handle();
        error_state_ = error_state_ || !no_write_error;
        handle_ = new_handle;
        return error_code::none;
    }

    [[nodiscard]] error_code out_file::allocate_buffer(std::size_t size) noexcept {
        // unbuffered mode is indicated by buffer_size == 0
        if (size == 0) {
            deallocate_buffer();
            return error_code::none;
        }

        if (size < min_buffer_size_) size = min_buffer_size_;
        else if (size > max_buffer_size_) size = max_buffer_size_;

        if (size == buffer_size_) {
            FSTLOG_ASSERT(buffer_ != nullptr && buffer_pos_ == 0);
            buffer_pos_ = 0;
            return error_code::none; // already allocated with the same size
        }

        if (memory_resource_ == nullptr) return error_code::mem_res_null;
        
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
        buffer_pos_ = 0;
        return error_code::none;
    }

    void out_file::deallocate_buffer() noexcept {
        if (buffer_ != nullptr) {
            aligned_nothrow_deallocate(
                buffer_,
                memory_resource_,
                buffer_size_,
                constants::cache_ls_nosharing);
            buffer_ = nullptr;
        }
        buffer_size_ = 0;
        buffer_pos_ = 0;
    }

    void out_file::steal_from(out_file& other) noexcept {
        if (this == &other) return;
        FSTLOG_ASSERT(handle_ == bad_handle_ && buffer_ == nullptr 
            && buffer_size_ == 0 && buffer_pos_ == 0);
        handle_ = other.handle_;
        buffer_ = other.buffer_;
        buffer_size_ = other.buffer_size_;
        buffer_pos_ = other.buffer_pos_;
        error_state_ = other.error_state_;
        memory_resource_ = other.memory_resource_;
        other.handle_ = bad_handle_;
        other.buffer_ = nullptr;
        other.buffer_size_ = 0;
        other.buffer_pos_ = 0;
        other.error_state_ = false;
        // we do not reset the memory_resource_ of the other object 
        // to prevent errors using it after move (it is not a dangling pointer, but the user should not use it after move)
    }
}
