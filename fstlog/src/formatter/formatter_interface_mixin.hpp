//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <formatter/formatter_interface.hpp>
#include <detail/make_allocated.hpp>

namespace fstlog {
	class formatter;
	template<class L>
    class formatter_interface_mixin final :
        public L,
        public formatter_interface
    {
    public:
        using allocator_type = typename L::allocator_type;

	private:
		using wrapper_type = formatter;
		formatter_interface_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(formatter_interface_mixin(allocator_type{})))
			: formatter_interface_mixin(allocator_type{}) {}
		explicit formatter_interface_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
            : L(allocator) {}

		formatter_interface_mixin(const formatter_interface_mixin& other) noexcept(
			noexcept(formatter_interface_mixin::get_allocator())
			&& noexcept(formatter_interface_mixin(formatter_interface_mixin{}, allocator_type{})))
            : formatter_interface_mixin(other, other.get_allocator()) {}
		formatter_interface_mixin(const formatter_interface_mixin& other, allocator_type const& allocator) noexcept(
			noexcept(L(formatter_interface_mixin{}, allocator_type{})))
            : L(other, allocator) {}
		formatter_interface_mixin& operator=(const formatter_interface_mixin&) = delete;
		formatter_interface_mixin(formatter_interface_mixin&&) = delete;
        formatter_interface_mixin& operator=(formatter_interface_mixin&&) = delete;
		~formatter_interface_mixin() = default;

	public:
		buff_span format_message(
			buff_span_const in,
			buff_span out) noexcept final
		{
			return L::format_message(in, out);
		}

		// allocates and constructs a new type erased formatter object
		const char* clone(wrapper_type& out) const noexcept final {
			return clone( out, L::get_allocator() );
		}

		// allocates and constructs a new type erased formatter object
		const char* clone(
			wrapper_type& out,
			allocator_type const& allocator) const noexcept final
		{
			out = make_allocated<formatter_interface_mixin<L>>(allocator, *this);
			if (out.pimpl() == nullptr) return "Allocation failed (formatter obj.)!";
			else return nullptr;
		}

        bool use() noexcept final {
            return L::use();
        }

        void release() noexcept final {
            L::release();
        }
		
	private:
		void add_reference() noexcept final {
			L::add_reference();
		}

		void release_referred() noexcept final {
			if (L::remove_reference()) {
				const auto allocator{ L::get_allocator() };
				this->~formatter_interface_mixin();
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
