//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once

#include <detail/mixin/allocator_mixin.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <filter/filter_mixin.hpp>

namespace fstlog {
	template<class T>
	auto make_allocated(fstlog_allocator const& allocator) noexcept;
	template<class T, class... Args>
	auto make_allocated(fstlog_allocator const& allocator,
		Args&&... args) noexcept;

	class filter_impl 
		: public filter_mixin<
				allocator_mixin> {
	private:
		using wrapper_type = filter_impl*;

		filter_impl() noexcept(
			noexcept(allocator_type())
			&& noexcept(filter_mixin(allocator_type{})))
			: filter_impl(allocator_type{}) {}
		explicit filter_impl(allocator_type const& allocator) noexcept(
			noexcept(filter_mixin(allocator_type{})))
			: filter_mixin(allocator) {}

		filter_impl(const filter_impl& other) noexcept(
			noexcept(this->get_allocator())
			&& noexcept(filter_mixin(filter_impl{}, allocator_type{})))
			: filter_impl(other, other.get_allocator()) {}
		filter_impl(const filter_impl& other, allocator_type const& allocator) noexcept(
			noexcept(filter_mixin(filter_impl{}, allocator_type{})))
			: filter_mixin(other, allocator) {}
		filter_impl& operator=(const filter_impl&) = delete;
		filter_impl(filter_impl&&) = delete;
		filter_impl& operator=(filter_impl&&) = delete;
		~filter_impl() = default;
			
		template<class T>
		friend auto make_allocated(
			fstlog_allocator const& allocator) noexcept;
		template<class T, class... Args>
		friend auto make_allocated(
			fstlog_allocator const& allocator,
			Args&&... args) noexcept;
		friend class filter;
	};
}
