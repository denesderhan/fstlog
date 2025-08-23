//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <cstdint>
#include <vector>
#include <thread>

#include <fstlog/core.hpp>
#ifndef FSTLOG_NOEXCEPTIONS
#include <test_mem_resource.hpp>

static void test_thread(std::vector<fstlog::core> &cores) {
    for (auto& c : cores) {
        c.detail_tls_buffer() = c.detail_get_buffer(1024);
    }
}
#endif

TEST_CASE("core") {
    SECTION("id") {
        fstlog::core core_1{ "core_1" };
        fstlog::core core_2{ "core_2" };
        fstlog::core core_3{ nullptr };
        fstlog::core core_4{ nullptr };
        CHECK(core_1.id() != core_2.id());
        CHECK(core_1.id() != core_3.id());
        CHECK(core_3.id() == core_4.id());

        core_3 = fstlog::core{ "core_3" };
        CHECK(core_3.id() != core_4.id());
        CHECK(core_3.id() != core_2.id());
        
        core_4 = core_3;
        CHECK(core_3.pimpl() == core_4.pimpl());
    };

    SECTION("core_limit") {    
        std::vector<fstlog::core> cores;
        CHECK(fstlog::core::limit() >= 4);
        CHECK(fstlog::core::limit() <= 128);
        while (cores.size() < fstlog::core::limit()) cores.push_back(fstlog::core{});
        for (auto& c : cores) {
            CHECK(c.good());
        }
#ifdef FSTLOG_NOEXCEPTIONS
        fstlog::core bad_core;
        CHECK(!bad_core.good());
        CHECK(bad_core.pimpl() == nullptr);
        CHECK(bad_core.id() == 0);
#else
        CHECK_THROWS_WITH(fstlog::core{}, fstlog::error_message(fstlog::error_code::core_limit));
#endif
    };

    SECTION("memory_leak") {
#ifndef FSTLOG_NOEXCEPTIONS
        if constexpr (std::is_same_v<fstlog::fstlog_allocator, std::pmr::polymorphic_allocator<unsigned char>>) {
            test_mem_resource test_res;
            fstlog::fstlog_allocator allocator(&test_res);

            std::vector<fstlog::core> cores;
            CHECK(fstlog::core::limit() >= 4);
            CHECK(fstlog::core::limit() <= 128);
            cores.push_back(fstlog::core("core_1", allocator));
            cores.push_back(fstlog::core("core_2", allocator));
            cores.push_back(fstlog::core("core_3", allocator));
            cores.push_back(fstlog::core("core_4", allocator));
            while (cores.size() < fstlog::core::limit()) cores.push_back(fstlog::core(allocator));
            CHECK(cores.size() == fstlog::core::limit());

            for (auto& c : cores) {
                CHECK(c.good());
                CHECK(c.pimpl() != nullptr);
            }

            // try to add 1 more than the limit (testing if failed core_impl.init() leaks)
            try {
                cores.push_back(fstlog::core("bad_core", allocator));
            }
            catch (...) {}
            CHECK(cores.size() == fstlog::core::limit());

            cores[0] = std::move(cores[1]);
            cores[2] = fstlog::core(cores[3]);
            cores[1] = fstlog::core("core_new_1", allocator);
            CHECK(cores[0].good());
            CHECK(cores[1].good());
            CHECK(cores[2].good());
            CHECK(cores[3].good());
            CHECK(cores[0].name() == "core_2");
            CHECK(cores[1].name() == "core_new_1");
            CHECK(cores[2].name() == "core_4");
            CHECK(cores[3].name() == "core_4");

            cores.push_back(fstlog::core("core_new_2", allocator));
            CHECK(cores.back().good());

            std::vector<std::thread> threads;
            for (int i = 0; i < 10; i++) {
                threads.push_back(std::thread(test_thread, std::ref(cores)));
            }
            for (auto& t : threads) t.join();

            std::intmax_t min_allocations = cores.size() * 3 + 10 * cores.size();

            // destroy all cores
            // all allocations will be deallocated
            cores.clear();

            CHECK(test_res.sum_allocations() >= min_allocations);
            CHECK(test_res.all_clear());
        }
        else {
            SKIP("Memory leak tests are implemented using std::pmr::memory_resource but fstlog::fstlog_allocator is set to different type!");
        }
#else
        SKIP("Memory leak tests are implemented using std::pmr::memory_resource that uses exceptions but exceptions are disabled!");
#endif
    };
}
