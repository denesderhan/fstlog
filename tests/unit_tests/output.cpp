//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>
#include <sstream>

#include <fstlog/output/output_console.hpp>
#include <fstlog/output/output_file.hpp>
#include <fstlog/output/output_null.hpp>
#include <fstlog/output/output_stream.hpp>
#include <fstlog/output/output_stream_mt.hpp>

#include <test_mem_resource.hpp>

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
            auto error = fstlog::output_file(out, "//////", fstlog::get_default_resource());
            CHECK(error == fstlog::error_code::file_create_fail);
            CHECK(!out.good());
            CHECK(out.pimpl() == nullptr);
            error = fstlog::output_stream(out, std::shared_ptr<std::stringstream>{}, fstlog::get_default_resource());
            CHECK(error == fstlog::error_code::obj_null);
            CHECK(!out.good());
            CHECK(out.pimpl() == nullptr);
            error = fstlog::output_stream_mt(
                out, std::shared_ptr<std::stringstream>{}, std::shared_ptr<std::mutex>{}, fstlog::get_default_resource());
            CHECK(error == fstlog::error_code::obj_null);
            CHECK(!out.good());
            CHECK(out.pimpl() == nullptr);
            error = fstlog::output_stream_mt(
                out, sstream1, std::shared_ptr<std::mutex>{}, fstlog::get_default_resource());
            CHECK(error == fstlog::error_code::obj_null);
            CHECK(!out.good());
            CHECK(out.pimpl() == nullptr);
            error = fstlog::output_stream_mt(
                out, std::shared_ptr<std::stringstream>{}, mutex, fstlog::get_default_resource());
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
            outputs.push_back(fstlog::output_file("test1.log"));
            outputs.push_back(fstlog::output_file("test2.log"));
            outputs.push_back(fstlog::output_null()); //8
            outputs.push_back(fstlog::output_null());
            outputs.push_back(fstlog::output_stream(sstream1));
            outputs.push_back(fstlog::output_stream(sstream2));
            outputs.push_back(fstlog::output_stream_mt(sstream3, mutex));
            outputs.push_back(fstlog::output_stream_mt(sstream3, mutex)); //13

            for (auto& o : outputs) {
                CHECK(o.good());
                CHECK(o.pimpl() != nullptr);
            }
            REQUIRE(outputs.size() == 14);
            CHECK(outputs[0].pimpl() != outputs[1].pimpl());
            CHECK(outputs[2].pimpl() != outputs[3].pimpl());
            CHECK(outputs[4].pimpl() != outputs[5].pimpl());
            CHECK(outputs[6].pimpl() != outputs[7].pimpl());
            CHECK(outputs[8].pimpl() != outputs[9].pimpl());
            CHECK(outputs[10].pimpl() != outputs[11].pimpl());
            CHECK(outputs[12].pimpl() != outputs[13].pimpl());

            // assignment
            outputs[0] = outputs[10];
            CHECK(outputs[0].pimpl() == outputs[10].pimpl());

            // copy constructor
            auto output_cp(outputs[6]);
            CHECK(outputs[6].good());
            CHECK(output_cp.good());
            CHECK(outputs[6].pimpl() == output_cp.pimpl());

            // move constructor
            auto o9_ptr = reinterpret_cast<std::uintptr_t>(outputs[7].pimpl());
            auto output_mv(std::move(outputs[7]));
            CHECK(outputs[7].pimpl() == nullptr);
            CHECK(reinterpret_cast<std::uintptr_t>(output_mv.pimpl()) == o9_ptr);
        }        
    };
    
    SECTION("memory_leak") {
        if constexpr (std::is_same_v<fstlog::memory_resource, std::pmr::memory_resource>) {
            auto sstream1{ std::make_shared<std::stringstream>() };
            auto sstream2{ std::make_shared<std::stringstream>() };
            auto sstream3{ std::make_shared<std::stringstream>() };
            auto mutex{ std::make_shared<std::mutex>() };
            test_mem_resource test_res;
            fstlog::memory_resource* res_ptr =
                reinterpret_cast<fstlog::memory_resource*>(&test_res);
            // error
            fstlog::output out;
            CHECK(!out.good());
            CHECK(out.pimpl() == nullptr);
            auto error =  fstlog::output_file(out, "//////", res_ptr);
            CHECK(error == fstlog::error_code::file_create_fail);
            CHECK(!out.good());
            CHECK(out.pimpl() == nullptr);
            error = fstlog::output_stream(out, std::shared_ptr<std::stringstream>{},res_ptr);
            CHECK(error == fstlog::error_code::obj_null);
            CHECK(!out.good());
            CHECK(out.pimpl() == nullptr);
            error = fstlog::output_stream_mt(
                out, std::shared_ptr<std::stringstream>{}, std::shared_ptr<std::mutex>{},res_ptr);
            CHECK(error == fstlog::error_code::obj_null);
            CHECK(!out.good());
            CHECK(out.pimpl() == nullptr);
            error = fstlog::output_stream_mt(
                out, sstream1, std::shared_ptr<std::mutex>{},res_ptr);
            CHECK(error == fstlog::error_code::obj_null);
            CHECK(!out.good());
            CHECK(out.pimpl() == nullptr);
            error = fstlog::output_stream_mt(
                out, std::shared_ptr<std::stringstream>{}, mutex,res_ptr);
            CHECK(error == fstlog::error_code::obj_null);
            CHECK(!out.good());
            CHECK(out.pimpl() == nullptr);

            // construct_assign_copy_move
            std::vector<fstlog::output> outputs;
            outputs.push_back(fstlog::output_console(res_ptr)); //0
            outputs.push_back(fstlog::output_cout(res_ptr));
            outputs.push_back(fstlog::output_cerr(res_ptr));
            outputs.push_back(fstlog::output_cerr(res_ptr));
            outputs.push_back(fstlog::output_clog(res_ptr));
            outputs.push_back(fstlog::output_clog(res_ptr)); //5
            outputs.push_back(fstlog::output_file("test1.log", res_ptr));
            outputs.push_back(fstlog::output_file("test2.log", res_ptr));
            outputs.push_back(fstlog::output_null(res_ptr)); //8
            outputs.push_back(fstlog::output_null(res_ptr));
            outputs.push_back(fstlog::output_stream(sstream1, res_ptr));
            outputs.push_back(fstlog::output_stream(sstream2, res_ptr));
            outputs.push_back(fstlog::output_stream_mt(sstream3, mutex, res_ptr));
            outputs.push_back(fstlog::output_stream_mt(sstream3, mutex, res_ptr)); //13

            for (auto& o : outputs) {
                CHECK(o.good());
                CHECK(o.pimpl() != nullptr);
            }
            REQUIRE(outputs.size() == 14);
            CHECK(outputs[0].pimpl() != outputs[1].pimpl());
            CHECK(outputs[2].pimpl() != outputs[3].pimpl());
            CHECK(outputs[4].pimpl() != outputs[5].pimpl());
            CHECK(outputs[6].pimpl() != outputs[7].pimpl());
            CHECK(outputs[8].pimpl() != outputs[9].pimpl());
            CHECK(outputs[10].pimpl() != outputs[11].pimpl());
            CHECK(outputs[12].pimpl() != outputs[13].pimpl());

            // assignment
            outputs[0] = outputs[10];
            CHECK(outputs[0].pimpl() == outputs[10].pimpl());

            // copy constructor
            auto output_cp(outputs[6]);
            CHECK(outputs[6].good());
            CHECK(output_cp.good());
            CHECK(outputs[6].pimpl() == output_cp.pimpl());

            // move constructor
            auto o9_ptr = reinterpret_cast<std::uintptr_t>(outputs[7].pimpl());
            auto output_mv(std::move(outputs[7]));
            CHECK(outputs[7].pimpl() == nullptr);
            CHECK(reinterpret_cast<std::uintptr_t>(output_mv.pimpl()) == o9_ptr);
        }
        else {
            SKIP("Memory leak tests are implemented using std::pmr::memory_resource but fstlog::memory_resource is set to different type!");
        }
    };
}

