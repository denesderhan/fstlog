//Copyright © 2022-present, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

 
#include <fstlog/logger/detail/log_arg_size.hpp>

TEST_CASE("log_arg_size") {
    const std::array<char, 4> arr0{ 0,1,2,3 };
    CHECK(fstlog::detail::log_arg_size<decltype(arr0)>() == fstlog::detail::log_arg_size(arr0));
    const std::pair<char, bool> pair0{ 'C', true};
    CHECK(fstlog::detail::log_arg_size<decltype(pair0)>() == fstlog::detail::log_arg_size(pair0));
    const std::tuple<char, int, bool> tup0{ 'C',1, false};
    CHECK(fstlog::detail::log_arg_size<decltype(tup0)>() == fstlog::detail::log_arg_size(tup0));
    CHECK(fstlog::detail::log_arg_size<decltype("Hello!")>() == fstlog::detail::log_arg_size("Hello!"));
    CHECK(fstlog::detail::log_arg_size<decltype(0)>() == fstlog::detail::log_arg_size(0));
    const auto tup1 = std::make_tuple(42, true );
    CHECK(fstlog::detail::log_arg_size<decltype(tup1)>() == fstlog::detail::log_arg_size(tup1));

    const auto temp = std::make_tuple(std::pair<char, bool>{'C', true}, std::array<int, 4>{0,1,2,3}, 0.1);
    CHECK(fstlog::detail::log_arg_size<decltype(temp)>() == fstlog::detail::log_arg_size(temp));
}
