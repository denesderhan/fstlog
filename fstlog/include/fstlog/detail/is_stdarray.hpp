//Copyright © 2022-present, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstddef>
#include <type_traits>

#include <fstlog/detail/rm_cvref_t.hpp>

namespace fstlog::detail {
    template<class T>
    struct is_stdarray_impl : std::false_type {};

    template<class T, std::size_t N>
    struct is_stdarray_impl<std::array<T, N>> : std::true_type {};

    template <typename T>
    struct is_stdarray : is_stdarray_impl<rm_cvref_t<T>> {};

    template <typename T>
    constexpr bool is_stdarray_v = is_stdarray<T>::value;
} // namespace fstlog::detail
