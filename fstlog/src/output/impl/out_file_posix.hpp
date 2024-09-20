//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdio>

#include <detail/nothrow_allocate.hpp>
#include <fstlog/detail/constants.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#include <detail/utf_conv.hpp>
#endif
namespace fstlog {
	template<class Allocator>
    class out_file_posix {
    public:
        using allocator_type = Allocator;

        out_file_posix() noexcept : out_file_posix(allocator_type{}) {}
        explicit out_file_posix(allocator_type const& allocator) noexcept
            : allocator_{ allocator } {}

        out_file_posix(const out_file_posix& other) = delete;
        out_file_posix(out_file_posix&& other) = delete;
        out_file_posix& operator=(const out_file_posix& rhs) = delete;
        out_file_posix& operator=(out_file_posix&& rhs) = delete;
        ~out_file_posix() noexcept {
            close();
            deallocate_buffer();
        }

        const char* open(
            const char* file_path,
            bool truncate,
            std::size_t buffer_size) noexcept
        {
			if (file_path == nullptr) return "Path nullptr!";
			if (handle_ != nullptr) return "Output already opened!";

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
			const unsigned char* path_begin{ safe_reinterpret_cast<const unsigned char*>(file_path) };
			const auto p_end{ path_begin + strlen(file_path) };
			wchar_t path[512]{ 0 };
			std::size_t char_num{ 1000 };
			//511 (last char must be '/0')
			const auto result = detail::utf8_to_utf16<char, wchar_t>(path_begin, p_end, path, path + 511, char_num);
			//Path was too long!
			if (result.ec != error_code::none) return "Path invalid or too long!";
#endif
			//Win32 api: Allowable range : 2 <= size <= INT_MAX(2147483647) 
			if (buffer_size == 1) buffer_size = 2;
			else if (buffer_size > 2147483647) buffer_size = 2147483647;
            //on close buffer is not deallocated (reused if opened with same size)
			if (buffer_size != buffer_size_) {
				deallocate_buffer();
				if (buffer_size != 0) {
					buffer_ = static_cast<char*>(aligned_nothrow_allocate(
						allocator_,
						buffer_size,
						constants::cache_ls_nosharing));
					if (buffer_ == nullptr) {
						return "Allocation failed (output buffer)!";
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
				handle_ = fopen(file_path, "wb");
			}
			else { 
				handle_ = fopen(file_path, "ab"); 
			}
#endif
			//buffer wont be deallocated
			if (handle_ == nullptr) return "Opening file path failed (invalid)!";
			
			const auto buffer_mode = buffer_size_ == 0 ? _IONBF : _IOFBF;
			if (setvbuf(handle_, buffer_, buffer_mode, buffer_size_)) {
				close();
				return "Setting file buffer failed!";
			}
			return nullptr;
        }

        void try_reopen(const char* file_path) noexcept {
			FSTLOG_ASSERT(handle_ != nullptr);
			
			FILE* temp{ nullptr };
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
			const auto p_end{ file_path + strlen(file_path) };
			wchar_t path[512]{ 0 };
			std::size_t char_num{ 1000 };
			//511 (last char must be '/0')
			const auto result = detail::utf8_to_utf16(file_path, p_end, path, path + 511, char_num);
			//Path was too long!
			if (result.ec != error_code::none) return;
			_wfopen_s(&temp, path, L"ab");
#else
			temp = fopen(file_path, "ab");
#endif
			if (temp == nullptr) return;

			fclose(handle_);

			const auto buffer_mode = buffer_size_ == 0 ? _IONBF : _IOFBF;
			//trying to set buffer_
			if (setvbuf(temp, buffer_, buffer_mode, buffer_size_)) {
				deallocate_buffer();
				//if fails trying to set unbuffered mode
				//if fails buffering will be default on the system
				setvbuf(temp, nullptr, _IONBF, 0);
			}
	
			handle_ = temp;

			// The setvbuf function may be used only after the stream pointed to by stream has
			// been associated with an open file and before any other operation(other than an
			// unsuccessful call to setvbuf) is performed on the stream.
        }

        void close() noexcept {
            if (handle_ != nullptr) {
                fclose(handle_);
                handle_ = nullptr;
            }
        }

        void flush() noexcept {
			FSTLOG_ASSERT(handle_ != nullptr);
			fflush(handle_);
        }

        void write(const char* data, std::size_t byte_size) noexcept {
			FSTLOG_ASSERT(handle_ != nullptr);
			fwrite(data, byte_size, 1, handle_);
        }

    private:
        void deallocate_buffer() noexcept {
            if (buffer_ != nullptr) {
				aligned_nothrow_deallocate(
					buffer_,
					allocator_,
					buffer_size_,
					constants::cache_ls_nosharing);
				buffer_ = nullptr;
            }
            buffer_size_ = 0;
        }

        FILE* handle_{ nullptr };
        char* buffer_{ nullptr };
        std::size_t buffer_size_{ 0 };
        allocator_type allocator_;
    };
}
