//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>
#include <sstream>

#include <fstlog/output/output_console.hpp>
#include <fstlog/output/output_cstream.hpp>
#include <fstlog/output/output_file.hpp>
#include <fstlog/output/output_null.hpp>
#include <fstlog/output/output_stream.hpp>
#include <fstlog/output/output_stream_mt.hpp>
#ifdef FSTLOG_ALLOCATOR_IS_STDPMR
#include <test_mem_resource.hpp>
#endif

TEST_CASE("output") {
    SECTION("construct") {
        SECTION("error") {
            auto sstream1{ std::make_shared<std::stringstream>() };
            auto sstream2{ std::make_shared<std::stringstream>() };
            auto sstream3{ std::make_shared<std::stringstream>() };
            auto mutex{ std::make_shared<std::mutex>() };
            
            fstlog::output out;
            CHECK(!out.good());
            CHECK(out.pimpl() == nullptr);
            auto error = fstlog::output_cstream(out, NULL, fstlog::fstlog_allocator{});
            CHECK(error == fstlog::error_code::stream_bad);
            CHECK(!out.good());
            CHECK(out.pimpl() == nullptr);
            error = fstlog::output_file(out, "//////", fstlog::fstlog_allocator{});
            CHECK(error == fstlog::error_code::path_bad);
            CHECK(!out.good());
            CHECK(out.pimpl() == nullptr);
            error = fstlog::output_stream(out, std::shared_ptr<std::stringstream>{}, fstlog::fstlog_allocator{});
            CHECK(error == fstlog::error_code::obj_null);
            CHECK(!out.good());
            CHECK(out.pimpl() == nullptr);
            error = fstlog::output_stream_mt(
                out, std::shared_ptr<std::stringstream>{}, std::shared_ptr<std::mutex>{}, fstlog::fstlog_allocator{});
            CHECK(error == fstlog::error_code::obj_null);
            CHECK(!out.good());
            CHECK(out.pimpl() == nullptr);
            error = fstlog::output_stream_mt(
                out, sstream1, std::shared_ptr<std::mutex>{}, fstlog::fstlog_allocator{});
            CHECK(error == fstlog::error_code::obj_null);
            CHECK(!out.good());
            CHECK(out.pimpl() == nullptr);
            error = fstlog::output_stream_mt(
                out, std::shared_ptr<std::stringstream>{}, mutex, fstlog::fstlog_allocator{});
            CHECK(error == fstlog::error_code::obj_null);
            CHECK(!out.good());
            CHECK(out.pimpl() == nullptr);
        };

        SECTION("construct_assign_copy_move") {
            auto sstream1{ std::make_shared<std::stringstream>() };
            auto sstream2{ std::make_shared<std::stringstream>() };
            auto sstream3{ std::make_shared<std::stringstream>() };
            auto mutex{ std::make_shared<std::mutex>() };
            std::vector<fstlog::output> outputs;
            outputs.push_back(fstlog::output_console()); //0
            outputs.push_back(fstlog::output_cout());
            outputs.push_back(fstlog::output_cerr());
            outputs.push_back(fstlog::output_cerr());
            outputs.push_back(fstlog::output_clog());
            outputs.push_back(fstlog::output_clog()); //5
            outputs.push_back(fstlog::output_cstream(stdout));
            outputs.push_back(fstlog::output_cstream(stderr));
            outputs.push_back(fstlog::output_file("test1.log"));
            outputs.push_back(fstlog::output_file("test2.log"));
            outputs.push_back(fstlog::output_null()); //10
            outputs.push_back(fstlog::output_null());
            outputs.push_back(fstlog::output_stream(sstream1));
            outputs.push_back(fstlog::output_stream(sstream2));
            outputs.push_back(fstlog::output_stream_mt(sstream3, mutex));
            outputs.push_back(fstlog::output_stream_mt(sstream3, mutex)); //15

            for (auto& o : outputs) {
                CHECK(o.good());
                CHECK(o.pimpl() != nullptr);
            }
            REQUIRE(outputs.size() == 16);
            CHECK(outputs[0].pimpl() != outputs[1].pimpl());
            CHECK(outputs[2].pimpl() != outputs[3].pimpl());
            CHECK(outputs[4].pimpl() != outputs[5].pimpl());
            CHECK(outputs[6].pimpl() != outputs[7].pimpl());
            CHECK(outputs[8].pimpl() != outputs[9].pimpl());
            CHECK(outputs[10].pimpl() != outputs[11].pimpl());
            CHECK(outputs[12].pimpl() != outputs[13].pimpl());
            CHECK(outputs[14].pimpl() != outputs[15].pimpl());

            // assignment
            outputs[0] = outputs[12];
            CHECK(outputs[0].pimpl() == outputs[12].pimpl());

            // move assignment
            auto o6_ptr = reinterpret_cast<std::uintptr_t>(outputs[6].pimpl());
            outputs[15] = std::move(outputs[6]);
            CHECK(outputs[6].pimpl() == nullptr);
            CHECK(reinterpret_cast<std::uintptr_t>(outputs[15].pimpl()) == o6_ptr);
            
            // copy constructor
            auto output_cp(outputs[8]);
            CHECK(outputs[8].good());
            CHECK(output_cp.good());
            CHECK(outputs[8].pimpl() == output_cp.pimpl());

            // move constructor
            auto o9_ptr = reinterpret_cast<std::uintptr_t>(outputs[9].pimpl());
            auto output_mv(std::move(outputs[9]));
            CHECK(outputs[9].pimpl() == nullptr);
            CHECK(reinterpret_cast<std::uintptr_t>(output_mv.pimpl()) == o9_ptr);
        }        
    };
    
    SECTION("memory_leak") {
#ifdef FSTLOG_ALLOCATOR_IS_STDPMR
        auto sstream1{ std::make_shared<std::stringstream>() };
        auto sstream2{ std::make_shared<std::stringstream>() };
        auto sstream3{ std::make_shared<std::stringstream>() };
        auto mutex{ std::make_shared<std::mutex>() };
        test_mem_resource test_res;
        fstlog::fstlog_allocator allocator(&test_res);
        // error
        fstlog::output out;
        CHECK(!out.good());
        CHECK(out.pimpl() == nullptr);
        auto error = fstlog::output_cstream(out, NULL, allocator);
        CHECK(error == fstlog::error_code::stream_bad);
        CHECK(!out.good());
        CHECK(out.pimpl() == nullptr);
        error = fstlog::output_file(out, "//////", allocator);
        CHECK(error == fstlog::error_code::path_bad);
        CHECK(!out.good());
        CHECK(out.pimpl() == nullptr);
        error = fstlog::output_stream(out, std::shared_ptr<std::stringstream>{}, allocator);
        CHECK(error == fstlog::error_code::obj_null);
        CHECK(!out.good());
        CHECK(out.pimpl() == nullptr);
        error = fstlog::output_stream_mt(
            out, std::shared_ptr<std::stringstream>{}, std::shared_ptr<std::mutex>{}, allocator);
        CHECK(error == fstlog::error_code::obj_null);
        CHECK(!out.good());
        CHECK(out.pimpl() == nullptr);
        error = fstlog::output_stream_mt(
            out, sstream1, std::shared_ptr<std::mutex>{}, allocator);
        CHECK(error == fstlog::error_code::obj_null);
        CHECK(!out.good());
        CHECK(out.pimpl() == nullptr);
        error = fstlog::output_stream_mt(
            out, std::shared_ptr<std::stringstream>{}, mutex, allocator);
        CHECK(error == fstlog::error_code::obj_null);
        CHECK(!out.good());
        CHECK(out.pimpl() == nullptr);

        // construct_assign_copy_move
        std::vector<fstlog::output> outputs;
        outputs.push_back(fstlog::output_console(allocator)); //0
        outputs.push_back(fstlog::output_cout(allocator));
        outputs.push_back(fstlog::output_cerr(allocator));
        outputs.push_back(fstlog::output_cerr(allocator));
        outputs.push_back(fstlog::output_clog(allocator));
        outputs.push_back(fstlog::output_clog(allocator)); //5
        outputs.push_back(fstlog::output_cstream(stdout, allocator));
        outputs.push_back(fstlog::output_cstream(stderr, allocator));
        outputs.push_back(fstlog::output_file("test1.log", allocator));
        outputs.push_back(fstlog::output_file("test2.log", allocator));
        outputs.push_back(fstlog::output_null(allocator)); //10
        outputs.push_back(fstlog::output_null(allocator));
        outputs.push_back(fstlog::output_stream(sstream1, allocator));
        outputs.push_back(fstlog::output_stream(sstream2, allocator));
        outputs.push_back(fstlog::output_stream_mt(sstream3, mutex, allocator));
        outputs.push_back(fstlog::output_stream_mt(sstream3, mutex, allocator)); //15

        for (auto& o : outputs) {
            CHECK(o.good());
            CHECK(o.pimpl() != nullptr);
        }
        REQUIRE(outputs.size() == 16);
        CHECK(outputs[0].pimpl() != outputs[1].pimpl());
        CHECK(outputs[2].pimpl() != outputs[3].pimpl());
        CHECK(outputs[4].pimpl() != outputs[5].pimpl());
        CHECK(outputs[6].pimpl() != outputs[7].pimpl());
        CHECK(outputs[8].pimpl() != outputs[9].pimpl());
        CHECK(outputs[10].pimpl() != outputs[11].pimpl());
        CHECK(outputs[12].pimpl() != outputs[13].pimpl());
        CHECK(outputs[14].pimpl() != outputs[15].pimpl());

        // assignment
        outputs[0] = outputs[12];
        CHECK(outputs[0].pimpl() == outputs[12].pimpl());

        // move assignment
        auto o6_ptr = reinterpret_cast<std::uintptr_t>(outputs[6].pimpl());
        outputs[15] = std::move(outputs[6]);
        CHECK(outputs[6].pimpl() == nullptr);
        CHECK(reinterpret_cast<std::uintptr_t>(outputs[15].pimpl()) == o6_ptr);

        // copy constructor
        auto output_cp(outputs[8]);
        CHECK(outputs[8].good());
        CHECK(output_cp.good());
        CHECK(outputs[8].pimpl() == output_cp.pimpl());

        // move constructor
        auto o9_ptr = reinterpret_cast<std::uintptr_t>(outputs[9].pimpl());
        auto output_mv(std::move(outputs[9]));
        CHECK(outputs[9].pimpl() == nullptr);
        CHECK(reinterpret_cast<std::uintptr_t>(output_mv.pimpl()) == o9_ptr);

#else 
        SKIP("Memory leak tests are implemented using std::pmr::memory_resource but fstlog::fstlog_allocator is set to different type!");
#endif
    };
}

