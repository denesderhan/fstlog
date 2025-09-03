//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>

#include <detail/unaligned_span.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <output/impl/out_file_posix.hpp>

namespace fstlog {
    template<class L>
    class out_file_mixin : public L
    {
    public:
        using memory_resource_type = typename L::memory_resource_type;
        
        explicit out_file_mixin(memory_resource_type* resource) noexcept(
            noexcept(L(nullptr))
            && noexcept(decltype(file_)(nullptr)))
            : L(resource),
            file_{ resource } {}

        out_file_mixin(const out_file_mixin& other) = delete;
        out_file_mixin(out_file_mixin&& other) = delete;
        out_file_mixin& operator=(const out_file_mixin& rhs) = delete;
        out_file_mixin& operator=(out_file_mixin&& rhs) = delete;
        
        ~out_file_mixin() = default;

        error_code init_output(
            const char* file_path,
            bool truncate,
            std::uint32_t buffer_size) noexcept
        {
            return file_.open(file_path, truncate, buffer_size);
        }

        void write_message(byte_span_const msg) noexcept {
            FSTLOG_ASSERT(msg.data_bytes() != nullptr);
            file_.write(safe_reinterpret_cast<const char*>(msg.data_bytes()), msg.size_bytes());
        }
        void flush() noexcept {
            file_.flush();
        }

    private:
        out_file_posix file_;
    };
}
