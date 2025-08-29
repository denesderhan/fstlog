#Copyright © 2022, Dénes Derhán.
#Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).

find_package(Threads REQUIRED)

set(helpers
    "helpers/test_mem_resource.hpp"
)

set(src
    "main.cpp"
)

set(src_unit_tests
    "unit_tests/core.cpp"
    "unit_tests/encoder_charconv_fast_mixin.cpp"
    "unit_tests/encoder_charconv_mixin.cpp"
    "unit_tests/encoder_helper.cpp"
    "unit_tests/encoder_stdformat_mixin.cpp"
    "unit_tests/encoder_timestamp_mixin.cpp"
    "unit_tests/error.cpp"
    "unit_tests/filter.cpp"
    "unit_tests/filter_internal.cpp"
    "unit_tests/format_str_helper.cpp"
    "unit_tests/formatter_txt_mixin.cpp"
    "unit_tests/internal_arg_header.cpp"
    "unit_tests/log_arg_size.cpp"
    "unit_tests/log_arg_size_compile_time.cpp"
    "unit_tests/log_buffer.cpp"
    "unit_tests/log_compute_msgsize_mixin.cpp"
    "unit_tests/log_element_counter_overhead.cpp"
    "unit_tests/log_level_filter_mixin.cpp"
    "unit_tests/log_noexcept.cpp"
    "unit_tests/log2_size.cpp"
    "unit_tests/logfield_formspec_fmt_mixin.cpp"
    "unit_tests/logfield_formspec_txt_fast_mixin.cpp"
    "unit_tests/logfield_formspec_txt_mixin.cpp"
    "unit_tests/logger.cpp"
    "unit_tests/logger_dropcount_mixin.cpp"
    "unit_tests/logger_dropcount_st_mixin.cpp"
    "unit_tests/logger_level_atomic_mixin.cpp"
    "unit_tests/logger_mt.cpp"
    "unit_tests/logger_st.cpp"
    "unit_tests/nearest_pow2.cpp"
    "unit_tests/output.cpp"
    "unit_tests/padded_size.cpp"
    "unit_tests/shift_fill.cpp"
    "unit_tests/small_string.cpp"
    "unit_tests/this_thread_id.cpp"
    "unit_tests/unaligned_span.cpp"
    "unit_tests/utf_conv.cpp"
    "unit_tests/valid_strftime_string.cpp"
)

set(ALL_FILES
    ${helpers}
    ${src}
    ${src_unit_tests}
)

source_group("helpers" FILES ${helpers})
source_group("src" FILES ${src})
source_group("src/unit_tests" FILES ${src_unit_tests})

add_executable(tests ${ALL_FILES})
set_target_properties(tests PROPERTIES 
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
)

target_include_directories(tests
    PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../fstlog/src>"
    PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/helpers>"
)

target_compile_definitions(tests PRIVATE
    "FSTLOG_DEBUG"
    "FSTLOG_TESTING"
)

################################################################################
# Dependencies
################################################################################

find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(tests
    PRIVATE Threads::Threads
    PRIVATE Catch2::Catch2
    PRIVATE fstlog
)

################################################################################
# Register tests with CTest
################################################################################
include(CTest)
include(Catch)
catch_discover_tests(tests)
