//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/api_def.hpp>
#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/fstlog_allocator.hpp>
#include <fstlog/detail/level.hpp>
#include <fstlog/detail/types.hpp>

namespace fstlog {
	class filter_impl;
	class filter {
	public:
		using allocator_type = fstlog_allocator;

		FSTLOG_API filter() noexcept(
			noexcept(allocator_type())
			&& noexcept(handle_error("")))
			: filter( allocator_type{} ) {}
		explicit FSTLOG_API filter(allocator_type const& allocator) noexcept(
			noexcept(handle_error("")))
		{
			[[maybe_unused]] const auto error = init(allocator);
			handle_error(error);
		}
		FSTLOG_API filter(
			level level,
			channel_type channel,
			allocator_type const& allocator = {}) noexcept(
				noexcept(handle_error("")))
		{
			[[maybe_unused]] const auto error = init(level, channel, allocator);
			handle_error(error);
		}
		FSTLOG_API filter(
			level level,
			channel_type first_channel,
			channel_type last_channel,
			allocator_type const& allocator = {}) noexcept(
				noexcept(handle_error("")))
		{
			[[maybe_unused]] const auto error = 
				init(level, first_channel, last_channel, allocator);
			handle_error(error);
		}
		FSTLOG_API filter(const filter& other) noexcept(
			noexcept(handle_error(""))) {
			[[maybe_unused]] const auto error = init(other);
			handle_error(error);
		}
		FSTLOG_API filter(
			const filter& other, 
			allocator_type const& allocator) noexcept(
				noexcept(handle_error(""))) 
		{
			[[maybe_unused]] const auto error = init(other, allocator);
			handle_error(error);
		}
		FSTLOG_API filter& operator=(const filter& other) noexcept;
		FSTLOG_API filter(filter&& other) noexcept;
		FSTLOG_API filter& operator=(filter&& other) noexcept;
		FSTLOG_API ~filter() noexcept;

		FSTLOG_API const char* init(allocator_type const& allocator) noexcept;
		FSTLOG_API const char* init(
			level level,
			channel_type channel,
			allocator_type const& allocator = {}) noexcept;
		FSTLOG_API const char* init(
			level level,
			channel_type first_channel,
			channel_type last_channel,
			allocator_type const& allocator = {}) noexcept;
		FSTLOG_API const char* init(const filter& other) noexcept;
		FSTLOG_API const char* init(const filter& other, allocator_type const& allocator) noexcept;
		
		FSTLOG_API bool good() const noexcept;
		
		FSTLOG_API void add_level(level level) noexcept;
		FSTLOG_API void add_level(level first, level last) noexcept;
		FSTLOG_API void add_channel(channel_type channel) noexcept;
		FSTLOG_API void add_channel(channel_type first, channel_type last) noexcept;
		FSTLOG_API bool filter_msg(level level, channel_type channel) const noexcept;

		FSTLOG_API filter(filter_impl* pimpl) noexcept;
		filter_impl* pimpl() const noexcept;
	private:
		filter_impl* pimpl_{ nullptr };
	};
}
