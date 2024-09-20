//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <detail/safe_reinterpret_cast.hpp>
#include <detail/byte_span.hpp>
#include <fstlog/detail/str_hash_fnv.hpp>

namespace fstlog {
    template<typename L>
    class hash_converter_null_mixin : public L {
    public:
        using allocator_type = typename L::allocator_type;

        hash_converter_null_mixin() noexcept(
			noexcept(allocator_type())
			&& noexcept(hash_converter_null_mixin(allocator_type{}))) 
			: hash_converter_null_mixin(allocator_type{}) {}
        explicit hash_converter_null_mixin(allocator_type const& allocator) noexcept(
			noexcept(L(allocator_type{})))
			: L(allocator) {}

        hash_converter_null_mixin(const hash_converter_null_mixin& other) noexcept(
			noexcept(hash_converter_null_mixin::get_allocator())
			&& noexcept(hash_converter_null_mixin(hash_converter_null_mixin{}, allocator_type{})))
            : hash_converter_null_mixin(other, other.get_allocator()) {}
		hash_converter_null_mixin(const hash_converter_null_mixin& other, allocator_type const& allocator) noexcept(
			noexcept(L(hash_converter_null_mixin{}, allocator_type{})))
            : L(other, allocator) {}

        hash_converter_null_mixin(hash_converter_null_mixin&& other) = delete;
        hash_converter_null_mixin& operator=(const hash_converter_null_mixin& rhs) = delete;
        hash_converter_null_mixin& operator=(hash_converter_null_mixin&& rhs) = delete;
        
        ~hash_converter_null_mixin() = default;

        static byte_span<const char> convert_hash(
			[[maybe_unused]] str_hash_fnv hash) noexcept 
		{
			constexpr auto temp{""};
			return byte_span<const char>{ temp, 0};
        }
    };
}
