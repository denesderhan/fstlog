//Copyright © 2022-present, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <type_traits>

#include <fstlog/detail/padded_size.hpp>

namespace fstlog {
    namespace detail {
        template<typename T, std::size_t padd_to>
        struct padded_t_padding {
            static constexpr std::size_t value =
            padded_size<padd_to>(sizeof(T))
                - sizeof(T);
        };

        template <typename T, std::size_t padding>
        struct padded_t_paddnonzero
        {
            using value_type = T;
            T value;
            unsigned char padding_bytes[padding]{ 0 };
            static constexpr std::size_t padded_data_size = sizeof(T) + padding;
        };

        template<typename T>
        struct padded_t_paddzero
        {
            using value_type = T;
            T value;
            static constexpr std::size_t padded_data_size = sizeof(T);
        };
    } // namespace fstlog::detail

    template<typename T, std::size_t padd_to>
    using padded_t = std::conditional_t<
        detail::padded_t_padding<T, padd_to>::value == 0,
        detail::padded_t_paddzero<T>,
        detail::padded_t_paddnonzero<T, detail::padded_t_padding<T, padd_to>::value>>;
} // namespace fstlog
