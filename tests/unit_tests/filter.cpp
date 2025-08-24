//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

#include <fstlog/filter/filter.hpp>
#ifndef FSTLOG_NOEXCEPTIONS
#include <test_mem_resource.hpp>
#endif

TEST_CASE("filter") {

    SECTION("construct") {
        fstlog::filter filt1;
        CHECK(filt1.good());
        CHECK(filt1.pimpl() != nullptr);
        const auto filt1_ptr = reinterpret_cast<std::uintptr_t>(filt1.pimpl());

        fstlog::filter filt2(filt1);
        CHECK(filt1.pimpl() != nullptr);
        CHECK(reinterpret_cast<std::uintptr_t>(filt1.pimpl()) == filt1_ptr);
        CHECK(filt2.good());
        CHECK(filt2.pimpl() != nullptr);
        CHECK(filt1.pimpl() != filt2.pimpl());

        fstlog::filter filt3(std::move(filt1));
        CHECK(filt1.pimpl() == nullptr);
        CHECK(!filt1.good());
        CHECK(filt3.good());
        CHECK(filt3.pimpl() != nullptr);
        CHECK(reinterpret_cast<std::uintptr_t>(filt3.pimpl()) == filt1_ptr);
        
    };

    SECTION("01") {
        fstlog::filter filt;
        filt.add_channel(0);
        filt.add_level(fstlog::level::Error);
        filt.add_level(fstlog::level::Trace);
        filt.add_level(fstlog::level::Fatal);

        CHECK(filt.filter_msg(fstlog::level::Trace, 0) == true);
        CHECK(filt.filter_msg(fstlog::level::Debug, 0) == false);
        CHECK(filt.filter_msg(fstlog::level::Info, 0) == false);
        CHECK(filt.filter_msg(fstlog::level::Warn, 0) == false);
        CHECK(filt.filter_msg(fstlog::level::Error, 0) == true);
        CHECK(filt.filter_msg(fstlog::level::Fatal, 0) == true);

        filt = fstlog::filter{};
        filt.add_channel(0);
        filt.add_level(fstlog::level::All, fstlog::level::None);
        CHECK(filt.filter_msg(fstlog::level::Trace, 0) == true);
        CHECK(filt.filter_msg(fstlog::level::Debug, 0) == true);
        CHECK(filt.filter_msg(fstlog::level::Info, 0) == true);
        CHECK(filt.filter_msg(fstlog::level::Warn, 0) == true);
        CHECK(filt.filter_msg(fstlog::level::Error, 0) == true);
        CHECK(filt.filter_msg(fstlog::level::Fatal, 0) == true);

        filt = fstlog::filter{};
        filt.add_channel(0);
        filt.add_level(fstlog::level::Debug, fstlog::level::Info);
        filt.add_level(fstlog::level::Info, fstlog::level::Error);
        CHECK(filt.filter_msg(fstlog::level::Trace, 0) == false);
        CHECK(filt.filter_msg(fstlog::level::Debug, 0) == true);
        CHECK(filt.filter_msg(fstlog::level::Info, 0) == true);
        CHECK(filt.filter_msg(fstlog::level::Warn, 0) == true);
        CHECK(filt.filter_msg(fstlog::level::Error, 0) == true);
        CHECK(filt.filter_msg(fstlog::level::Fatal, 0) == false);
    }

    SECTION("02") {
        fstlog::filter filt;

        filt.add_level(fstlog::level::All, fstlog::level::None);
        CHECK(filt.filter_msg(fstlog::level::Info, 255) == false);
        CHECK(filt.filter_msg(fstlog::level::Info, 0) == false);
        CHECK(filt.filter_msg(fstlog::level::Info, 250) == false);

        filt.add_channel(0);
        CHECK(filt.filter_msg(fstlog::level::Info, 0) == true);
        CHECK(filt.filter_msg(fstlog::level::Info, 1) == false);
        CHECK(filt.filter_msg(fstlog::level::Info, 255) == false);

        filt.add_channel(255);
        CHECK(filt.filter_msg(fstlog::level::Info, 255) == true);
        CHECK(filt.filter_msg(fstlog::level::Info, 254) == false);
        CHECK(filt.filter_msg(fstlog::level::Info, 0) == true);
        CHECK(filt.filter_msg(fstlog::level::Info, 1) == false);
        CHECK(filt.filter_msg(fstlog::level::Info, 200) == false);
    };

    SECTION("memory_leak") {
#ifndef FSTLOG_NOEXCEPTIONS
        if constexpr (std::is_same_v<fstlog::fstlog_allocator, std::pmr::polymorphic_allocator<unsigned char>>) {
            test_mem_resource test_res1;
            fstlog::fstlog_allocator allocator1(&test_res1);
            test_mem_resource test_res2;
            fstlog::fstlog_allocator allocator2(&test_res2);

            std::vector<fstlog::filter> filters1;
            for (fstlog::channel_type i = 0; i < 10; i++) filters1.push_back(fstlog::filter(fstlog::level::Info, 0, i, allocator1));
            std::vector<fstlog::filter> filters2;
            for (fstlog::channel_type i = 0; i < 10; i++) filters2.push_back(fstlog::filter(fstlog::level::Info, i, i + 10, allocator2));
            filters1[0] = filters2[0];
            filters2[1] = fstlog::filter(filters1[1]);
            filters1[1] = std::move(filters2[0]);
            filters2[2] = fstlog::filter(std::move(filters1[0]));
            filters1[2] = fstlog::filter{};
            filters2[3] = filters2[4];
            filters1[5] = filters1[6];


            filters1.clear();
            filters2.clear();
            CHECK(test_res1.all_clear());
            CHECK(test_res2.all_clear());
        }
        else {
            SKIP("Memory leak tests are implemented using std::pmr::memory_resource but fstlog::fstlog_allocator is set to different type!");
        }
#else
        SKIP("Memory leak tests are implemented using std::pmr::memory_resource that uses exceptions but exceptions are disabled!");
#endif
    }
}

