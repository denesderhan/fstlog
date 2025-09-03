//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <cstdint>
#include <vector>
#include <thread>

#include <fstlog/core.hpp>
#include <test_mem_resource.hpp>

TEST_CASE("core") {
    SECTION("id") {
        fstlog::core core_1{ "core_1" };
        fstlog::core core_2{ "core_2" };
        fstlog::core core_3{ nullptr };
        fstlog::core core_4{ nullptr };
        CHECK(core_1 != core_2);
        CHECK(core_1.id() != core_2.id());
        CHECK(core_1.id() != core_3.id());
        CHECK(core_3 == core_4);
        CHECK(core_3.id() == core_4.id());

        core_3 = fstlog::core{ "core_3" };
        CHECK(core_3.id() != core_4.id());
        CHECK(core_3.id() != core_2.id());
        
        core_4 = core_3;
        CHECK(core_3.pimpl() == core_4.pimpl());
        CHECK(core_4 == core_3);
    };

    // for concurrency testing to work a proper TSAN build is required!
    // for memory leak testing to work, the build has to use memory_resource == std::pmr!
    // compile with: -DFSTLOG_RESOURCE=./memory_resource/std_pmr
    SECTION("concurrency_and_memory_leak") {
        auto test_thread = [](std::vector<fstlog::core>& cores) {
                for (auto& c : cores) {
                    if (c.good()) {
                        c.detail_tls_buffer() = c.detail_get_buffer(0); // get default buffer size
                        CHECK(c.detail_tls_buffer().good());
                        if (c.detail_tls_buffer().good()) {
                            CHECK(c.detail_tls_buffer().size() != 0);
                        }
                    }
                }
            };
        
        fstlog::memory_resource* res_ptr = fstlog::get_default_resource();
        // for memory leak testing to work, memory_resource has to be std::pmr!
        test_mem_resource test_res;
        if constexpr (std::is_same_v<fstlog::memory_resource, std::pmr::memory_resource>) {
            // If the resource types are matching the reinterpret_cast is a no op.
            // we use the test resource to detect
            res_ptr = reinterpret_cast<fstlog::memory_resource*>(&test_res);
        }
        std::vector<fstlog::core> cores;
        
        // core limit
        {
            CHECK(fstlog::core::limit() >= 4);
            CHECK(fstlog::core::limit() <= 128);
            cores.push_back(fstlog::core("core_1", res_ptr));
            cores.push_back(fstlog::core("core_2", res_ptr));
            cores.push_back(fstlog::core("core_3", res_ptr));
            cores.push_back(fstlog::core("core_4", res_ptr));
            while (cores.size() < fstlog::core::limit()) cores.push_back(fstlog::core(res_ptr));
            CHECK(cores.size() == fstlog::core::limit());

            for (auto& c : cores) {
                CHECK(c.good());
                CHECK(c.pimpl() != nullptr);
            }

            // try to add 1 more than the limit (testing if failed core_impl.init() leaks)
#ifdef FSTLOG_NOEXCEPTIONS
            fstlog::core bad_core;
            CHECK(!bad_core.good());
            CHECK(bad_core.pimpl() == nullptr);
            CHECK(bad_core.id() == 0);
#else
            CHECK_THROWS_WITH(fstlog::core{}, fstlog::error_message(fstlog::error_code::core_limit));
#endif
        }

        // copy_move
        {
            cores[0] = std::move(cores[1]);
            cores[2] = fstlog::core(cores[3]);
            cores[1] = fstlog::core("core_new_1", res_ptr);
            CHECK(cores[0].good());
            CHECK(cores[1].good());
            CHECK(cores[2].good());
            CHECK(cores[3].good());
            CHECK(cores[0].name() == "core_2");
            CHECK(cores[1].name() == "core_new_1");
            CHECK(cores[2].name() == "core_4");
            CHECK(cores[3].name() == "core_4");

            // we shold be able to create one more "good" core
            // move made empty slots
            cores.push_back(fstlog::core("core_new_2", res_ptr));
            for (auto& c : cores) {
                CHECK(c.good());
            }
        }

        // concurrency
        {
            CHECK(!cores.empty());
            std::vector<std::thread> threads;
            for (int i = 0; i < 10; i++) {
                threads.push_back(std::thread(test_thread, std::ref(cores)));
            }

            // joining a thread releases the TLS log_buffers
            for (auto& t : threads) t.join();

            // destroy all cores
            // all allocations must be deallocated
            cores.clear();
        }

        // memory_leak
        {
            CHECK(cores.empty());
            if constexpr (std::is_same_v<fstlog::memory_resource, std::pmr::memory_resource>) {
                std::intmax_t min_allocations = cores.size() * 3 + 10 * cores.size();
                CHECK(test_res.sum_allocations() >= min_allocations);
                CHECK(test_res.all_clear());
            }
            else {
                SKIP("Memory leak tests are implemented using std::pmr::memory_resource but fstlog::memory_resource is set to different type!");
            }
        }
    };
}
