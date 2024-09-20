//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <sink/sink_interface.hpp>
#include <detail/nothrow_allocate.hpp>

namespace fstlog {
	class sink;
    template<class L>
    class sink_interface_mixin final :
        public L,
        public sink_interface
    {
        typedef std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> steady_msec;
    public:
        using allocator_type = typename L::allocator_type;
	private:
		using wrapper_type = sink;
		sink_interface_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(sink_interface_mixin(allocator_type{})))
			: sink_interface_mixin(allocator_type{}) {}
		explicit sink_interface_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
			: L(allocator) {}

		sink_interface_mixin(const sink_interface_mixin&) = delete;
		sink_interface_mixin& operator=(const sink_interface_mixin&) = delete;
		sink_interface_mixin(sink_interface_mixin&&) = delete;
		sink_interface_mixin& operator=(sink_interface_mixin&&) = delete;
		~sink_interface_mixin() = default;

	public:
        error_code sink_msg_block(const unsigned char* dat_ptr, std::uint32_t dat_size) noexcept final{
            return L::sink_msg_block(dat_ptr, dat_size);
        }
        bool needs_immediate_flush() const noexcept final{
            return L::needs_immediate_flush();
        }
        steady_msec next_flush_time() const noexcept final{
            return L::next_flush_time();
        }
        void flush(steady_msec current_time) noexcept final{
            L::flush(current_time);
        }        
        bool use() noexcept final{
            return L::use();
        }
        void release() noexcept final{
            L::release();
        }

	private:
		void add_reference() noexcept final {
			L::add_reference();
		}

		void release_referred() noexcept final {
			if (L::remove_reference()) {
				const auto allocator{ L::get_allocator() };
				this->~sink_interface_mixin();
				nothrow_deallocate(this, allocator);
			}
		}

		template<class T>
		friend auto make_allocated(
			fstlog_allocator const& allocator) noexcept;
		template<class T, class... Args>
		friend auto make_allocated(
			fstlog_allocator const& allocator,
			Args&&... args) noexcept;
		friend wrapper_type;
    };
}
