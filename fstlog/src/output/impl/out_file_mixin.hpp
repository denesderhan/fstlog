//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>

#include <detail/byte_span.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <output/impl/out_file_posix.hpp>

namespace fstlog {
    template<class L>
    class out_file_mixin : public L
    {
    public:
        using allocator_type = typename L::allocator_type;
        
		out_file_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(out_file_mixin(allocator_type{})))
			: out_file_mixin(allocator_type{}) {}
        explicit out_file_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{}))
			&& noexcept(decltype(file_)(allocator_type{})))
            : L(allocator),
			file_{ allocator } {}

        out_file_mixin(const out_file_mixin& other) = delete;
        out_file_mixin(out_file_mixin&& other) = delete;
        out_file_mixin& operator=(const out_file_mixin& rhs) = delete;
        out_file_mixin& operator=(out_file_mixin&& rhs) = delete;
        
        ~out_file_mixin() = default;

        auto init_output(
			const char* file_path,
            bool truncate,
            std::uint32_t buffer_size) noexcept
        {
            return file_.open(file_path, truncate, buffer_size);
        }

        void write_message(buff_span_const msg) noexcept {
			FSTLOG_ASSERT(msg.data() != nullptr);
			file_.write(safe_reinterpret_cast<const char*>(msg.data()), msg.size_bytes());
        }
        void flush() noexcept {
            file_.flush();
        }

    private:
        out_file_posix<allocator_type> file_;
    };
}
