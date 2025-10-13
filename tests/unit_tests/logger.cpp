//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <type_traits>

#include <fstlog/core.hpp>
#include <fstlog/logger/log_macro.hpp>
#include <fstlog/logger/logger.hpp>

TEST_CASE("logger") {
    //for each SECTION the TEST_CASE is executed from the start!
    fstlog::core core;
    core.poll_interval(std::chrono::milliseconds{0});


    SECTION("no_throw") {
        CHECK(std::is_nothrow_constructible_v<fstlog::logger_impl>);
        CHECK(std::is_nothrow_move_constructible_v<fstlog::logger_impl>);
        CHECK(std::is_nothrow_move_assignable_v<fstlog::logger_impl>);
        
#ifdef FSTLOG_NOEXCEPTIONS
        CHECK(noexcept(fstlog::handle_error(fstlog::error_code::none)));
        CHECK(std::is_nothrow_constructible_v<fstlog::logger,
            fstlog::core&,
            std::string_view&,
            fstlog::level&,
            fstlog::channel_type&>);
#else
        CHECK(!noexcept(fstlog::handle_error(fstlog::error_code::none)));
        CHECK(!std::is_nothrow_constructible_v<fstlog::logger,
            fstlog::core&, 
            std::string_view&, 
            fstlog::level&,
            fstlog::channel_type&>);
#endif
        CHECK(std::is_nothrow_move_constructible_v<fstlog::logger>);
        CHECK(std::is_nothrow_move_assignable_v<fstlog::logger>);
    };

    SECTION("construct") {
        // creating a logger with bad core (core.pimpl_ == nullptr)
        fstlog::logger logger(fstlog::core(nullptr));
        CHECK(logger.get_core().pimpl() == nullptr);
        CHECK(logger.name() == fstlog::small_string<32>{"Unnamed"});
        CHECK(logger.channel() == 1);
        CHECK(logger.thread() == fstlog::this_thread::get_str());
        CHECK(logger.dropped() == 0);
        CHECK(logger.level() == fstlog::level::All);
        
        CHECK(logger.good() == false);
        logger.set_core(core);
        CHECK(logger.get_core().pimpl() == core.pimpl());
        CHECK(logger.good() == false);
        logger.new_buffer(1024);
        CHECK(logger.good() == true);

        logger.set_name("logger_1");
        logger.set_thread("thread_1");
        logger.set_level(fstlog::level::Debug);
        logger.set_channel(10);
        CHECK(logger.name() == fstlog::small_string<32>{"logger_1"});
        CHECK(logger.channel() == 10);
        CHECK(logger.thread() == fstlog::small_string<32>{"thread_1"});
        CHECK(logger.level() == fstlog::level::Debug);

        for (int i = 0; i < 10; i++) {
            LOG_LL_FATAL(logger, "Test text, test text, test text, test text, test text, test text, test text.");
        }
        CHECK(logger.dropped() > 0);

        const auto dropped_logs{ logger.dropped() };
                
        CHECK(logger.get_core().pimpl() == core.pimpl());
        CHECK(logger.name() == fstlog::small_string<32>{"logger_1"});
        CHECK(logger.channel() == 10);
        CHECK(logger.thread() == fstlog::small_string<32>{"thread_1"});
        CHECK(logger.level() == fstlog::level::Debug);
        CHECK(logger.dropped() == dropped_logs);

        //move construct/assign
        fstlog::logger logger_mov{ std::move(logger) };
        
        CHECK(logger.get_core().pimpl() == nullptr);
        CHECK(logger.name() == fstlog::small_string<32>{"Unnamed"});
        CHECK(logger.channel() == 10);
        CHECK(logger.thread() == fstlog::small_string<32>{"thread_1"});
        CHECK(logger.level() == fstlog::level::Debug);
        CHECK(logger.dropped() == 0);
        
        CHECK(logger_mov.get_core().pimpl() == core.pimpl());
        CHECK(logger_mov.name() == fstlog::small_string<32>{"logger_1"});
        CHECK(logger_mov.channel() == 10);
        CHECK(logger_mov.thread() == fstlog::small_string<32>{"thread_1"});
        CHECK(logger_mov.level() == fstlog::level::Debug);
        CHECK(logger_mov.dropped() == dropped_logs);

        fstlog::logger logger_mov2(fstlog::core(nullptr));
        logger_mov2 = std::move(logger_mov);

        CHECK(logger_mov2.get_core().pimpl() == core.pimpl());
        CHECK(logger_mov2.name() == fstlog::small_string<32>{"logger_1"});
        CHECK(logger_mov2.channel() == 10);
        CHECK(logger_mov2.thread() == fstlog::small_string<32>{"thread_1"});
        CHECK(logger_mov2.level() == fstlog::level::Debug);
    };

    SECTION("multi_core") {
        fstlog::core temp_core0;
        auto first_id{ temp_core0.id() + 1 };
        
        for (auto i = 0; i < 10; i++) {
            fstlog::core temp_core;
            CHECK(temp_core.id() == first_id + i);
            fstlog::logger temp_logger(temp_core);
            CHECK(temp_logger.get_core().pimpl() == temp_core.pimpl());
            CHECK(temp_logger.good() == false);
            temp_logger.new_buffer(1024);
            CHECK(temp_logger.good() == true);
        }
    };
    
}
