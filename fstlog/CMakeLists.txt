#Copyright © 2022, Dénes Derhán.
#Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).

find_package(Threads REQUIRED)

set(include_fstlog
    "include/fstlog/compatible.hpp"
    "include/fstlog/core.hpp"
    "include/fstlog/version.hpp"
)

set(include_fstlog_detail
    "include/fstlog/detail/aggregate_type.hpp"
    "include/fstlog/detail/api_def.hpp"
    "include/fstlog/detail/array_concat.hpp"
    "include/fstlog/detail/array_copy.hpp"
    "include/fstlog/detail/char_type.hpp"
    "include/fstlog/detail/constants.hpp"
    "include/fstlog/detail/convert_to_basic_string_view.hpp"
    "include/fstlog/detail/error_handling.hpp"
    "include/fstlog/detail/fast_to_str.hpp"
    "include/fstlog/detail/fnv_hash.hpp"
    "include/fstlog/detail/fstlog_allocator.hpp"
    "include/fstlog/detail/fstlog_assert.hpp"
    "include/fstlog/detail/fstlog_ex.hpp"
    "include/fstlog/detail/has_input_iterator.hpp"
    "include/fstlog/detail/hash_type.hpp"
    "include/fstlog/detail/internal_arg_header.hpp"
    "include/fstlog/detail/internal_msg_header.hpp"
    "include/fstlog/detail/is_char_type.hpp"
    "include/fstlog/detail/is_pow2.hpp"
    "include/fstlog/detail/is_stdarray.hpp"
    "include/fstlog/detail/is_string_like.hpp"
    "include/fstlog/detail/is_tuple.hpp"
    "include/fstlog/detail/log_buffer.hpp"
    "include/fstlog/detail/log_element_type.hpp"
    "include/fstlog/detail/level.hpp"
    "include/fstlog/detail/log_metaargs.hpp"
    "include/fstlog/detail/log_policy.hpp"
    "include/fstlog/detail/log_type.hpp"
    "include/fstlog/detail/log_type_metadata.hpp"
    "include/fstlog/detail/log2_size.hpp"
    "include/fstlog/detail/noexceptions.hpp"
    "include/fstlog/detail/padded_size.hpp"
    "include/fstlog/detail/padded_t.hpp"
    "include/fstlog/detail/rm_cvref_t.hpp"
    "include/fstlog/detail/small_string.hpp"
    "include/fstlog/detail/str_hash_fnv.hpp"
    "include/fstlog/detail/type_signature.hpp"
    "include/fstlog/detail/types.hpp"
    "include/fstlog/detail/ut_cast.hpp"
)

set(include_fstlog_filter
    "include/fstlog/filter/filter.hpp"
)

set(include_fstlog_formatter
    "include/fstlog/formatter/formatter.hpp"
    "include/fstlog/formatter/formatter_null.hpp"
    "include/fstlog/formatter/formatter_stdformat.hpp"
    "include/fstlog/formatter/formatter_txt.hpp"
    "include/fstlog/formatter/formatter_txt_fast.hpp"
)

set(include_fstlog_logger
    "include/fstlog/logger/log_macro.hpp"
    "include/fstlog/logger/log_macro_nometa.hpp"
    "include/fstlog/logger/log_policy_guaranteed.hpp"
    "include/fstlog/logger/log_policy_lowlatency.hpp"
    "include/fstlog/logger/log_policy_nonguaranteed.hpp"
    "include/fstlog/logger/logger.hpp"
    "include/fstlog/logger/logger_mt.hpp"
    "include/fstlog/logger/logger_st.hpp"
    "include/fstlog/logger/logger_st_fix.hpp"
)

set(include_fstlog_logger_detail
    "include/fstlog/logger/detail/can_log_type.hpp"
    "include/fstlog/logger/detail/is_nested_type.hpp"
    "include/fstlog/logger/detail/log_arg_size.hpp"
    "include/fstlog/logger/detail/log_arg_size_compile_time.hpp"
    "include/fstlog/logger/detail/log_element_counter_overhead.hpp"
    "include/fstlog/logger/detail/log_element_payload_size.hpp"
    "include/fstlog/logger/detail/logger_base_mixin.hpp"
    "include/fstlog/logger/detail/logger_buffer_mixin.hpp"
    "include/fstlog/logger/detail/logger_buffer_thread_local_mixin.hpp"
    "include/fstlog/logger/detail/logger_channel_compile_time_mixin.hpp"
    "include/fstlog/logger/detail/logger_channel_mixin.hpp"
    "include/fstlog/logger/detail/logger_core_mixin.hpp"
    "include/fstlog/logger/detail/logger_dropcount_mixin.hpp"
    "include/fstlog/logger/detail/logger_dropcount_null_mixin.hpp"
    "include/fstlog/logger/detail/logger_dropcount_st_mixin.hpp"
    "include/fstlog/logger/detail/logger_level_atomic_mixin.hpp"
    "include/fstlog/logger/detail/logger_level_compile_time_mixin.hpp"
    "include/fstlog/logger/detail/logger_level_mixin.hpp"
    "include/fstlog/logger/detail/logger_msgsize_mixin.hpp"
    "include/fstlog/logger/detail/logger_msgsize_thread_local_mixin.hpp"
    "include/fstlog/logger/detail/logger_name_compile_time_mixin.hpp"
    "include/fstlog/logger/detail/logger_name_mixin.hpp"
    "include/fstlog/logger/detail/logger_thread_compile_time_mixin.hpp"
    "include/fstlog/logger/detail/logger_thread_mixin.hpp"
    "include/fstlog/logger/detail/logger_thread_query_mixin.hpp"
    "include/fstlog/logger/detail/logger_thread_thread_local_mixin.hpp"
    "include/fstlog/logger/detail/logger_writer_mixin.hpp"
    "include/fstlog/logger/detail/stamp_chrono_mixin.hpp"
    "include/fstlog/logger/detail/this_thread_id.hpp"
    "include/fstlog/logger/detail/this_thread_str.hpp"
    "include/fstlog/logger/detail/write_arg.hpp"
    "include/fstlog/logger/detail/write_var.hpp"
)

set(include_fstlog_logger_detail_log
    "include/fstlog/logger/detail/log/log_addmeta_mixin.hpp"
    "include/fstlog/logger/detail/log/log_check_buffer_mixin.hpp"
    "include/fstlog/logger/detail/log/log_check_core_mixin.hpp"
    "include/fstlog/logger/detail/log/log_compute_msgsize_mixin.hpp"
    "include/fstlog/logger/detail/log/log_level_filter_mixin.hpp"
    "include/fstlog/logger/detail/log/log_nobuffer_nolog_mixin.hpp"
    "include/fstlog/logger/detail/log/log_policy_fix_mixin.hpp"
    "include/fstlog/logger/detail/log/log_policy_mixin.hpp"
    "include/fstlog/logger/detail/log/log_try_get_buffer_mixin.hpp"
)

set(include_fstlog_output
    "include/fstlog/output/output.hpp"
    "include/fstlog/output/output_console.hpp"
    "include/fstlog/output/output_cstream.hpp"
    "include/fstlog/output/output_file.hpp"
    "include/fstlog/output/output_null.hpp"
    "include/fstlog/output/output_stream.hpp"
    "include/fstlog/output/output_stream_mt.hpp"
)

set(include_fstlog_sink
    "include/fstlog/sink/sink.hpp"
    "include/fstlog/sink/sink_null.hpp"
    "include/fstlog/sink/sink_sort.hpp"
    "include/fstlog/sink/sink_unsort.hpp"
)

set(src
    "src/config_buffer.hpp"
    "src/config_core.hpp"
    "src/config_formatter_txt.hpp"
    "src/config_parser.hpp"
    "src/config_sink.hpp"
    "src/core.cpp"
    "src/core_impl.cpp"
    "src/core_impl.hpp"
)

set(src_detail
    "src/detail/buffer_operation_result.hpp"
    "src/detail/byte_span.hpp"
    "src/detail/checked_iterator.hpp"
    "src/detail/dyn_array.hpp"
    "src/detail/dyn_buffer.hpp"
    "src/detail/error.hpp"
    "src/detail/error_code.hpp"
    "src/detail/log_buffer.cpp"
    "src/detail/log_buffer_impl.cpp"
    "src/detail/log_buffer_impl.hpp"
    "src/detail/log_buffer_unread_data.hpp"
    "src/detail/make_allocated.hpp"
    "src/detail/nearest_pow2.hpp"
    "src/detail/nothrow_allocate.hpp"
    "src/detail/safe_reinterpret_cast.hpp"
    "src/detail/utf_conv.hpp"
    "src/detail/utf8_helper.hpp"
)

set(src_detail_mixin
    "src/detail/mixin/allocator_mixin.hpp"
    "src/detail/mixin/concurrent_use_mixin.hpp"
    "src/detail/mixin/error_state_mixin.hpp"
    "src/detail/mixin/exclusive_use_mixin.hpp"
    "src/detail/mixin/mutex_external_mixin.hpp"
    "src/detail/mixin/mutex_internal_mixin.hpp"
    "src/detail/mixin/reference_counter_mixin.hpp"
)

set(src_filter
    "src/filter/filter.cpp"
    "src/filter/filter_impl.hpp"
    "src/filter/filter_internal.hpp"
    "src/filter/filter_mixin.hpp"
)

set(src_formatter
    "src/formatter/formatter.cpp"
    "src/formatter/formatter_interface.hpp"
    "src/formatter/formatter_interface_mixin.hpp"
    "src/formatter/formatter_null.cpp"
    "src/formatter/formatter_stdformat.cpp"
    "src/formatter/formatter_txt.cpp"
    "src/formatter/formatter_txt_fast.cpp"
)

set(src_formatter_impl
    "src/formatter/impl/arg_parser_mixin.hpp"
    "src/formatter/impl/formatter_null_mixin.hpp"
    "src/formatter/impl/formatter_txt_mixin.hpp"
    "src/formatter/impl/header_internal_mixin.hpp"
    "src/formatter/impl/input_span_mixin.hpp"
    "src/formatter/impl/logfield_formspec_fmt_mixin.hpp"
    "src/formatter/impl/logfield_formspec_txt_fast_mixin.hpp"
    "src/formatter/impl/logfield_formspec_txt_mixin.hpp"
    "src/formatter/impl/logfield_pos_mixin.hpp"
    "src/formatter/impl/output_span_mixin.hpp"
)

set(src_formatter_impl_detail
    "src/formatter/impl/detail/decoder_internal_mixin.hpp"
    "src/formatter/impl/detail/encoder_aggregate_separator_txt_mixin.hpp"
    "src/formatter/impl/detail/encoder_charconv_fast_mixin.hpp"
    "src/formatter/impl/detail/encoder_charconv_mixin.hpp"
    "src/formatter/impl/detail/encoder_stdformat_mixin.hpp"
    "src/formatter/impl/detail/format_setting_txt.hpp"
    "src/formatter/impl/detail/format_setting_txt_fast.hpp"
    "src/formatter/impl/detail/format_str_helper.hpp"
    "src/formatter/impl/detail/format_string_error.hpp"
    "src/formatter/impl/detail/hash_converter_null_mixin.hpp"
    "src/formatter/impl/detail/logfield.hpp"
    "src/formatter/impl/detail/nano_to_seconds_txt.hpp"
    "src/formatter/impl/detail/policy_txt.hpp"
    "src/formatter/impl/detail/severity_txt.hpp"
    "src/formatter/impl/detail/shift_fill.hpp"
    "src/formatter/impl/detail/time_string.hpp"
    "src/formatter/impl/detail/time_string_cache.hpp"
    "src/formatter/impl/detail/time_to_str_converter.hpp"
    "src/formatter/impl/detail/tz_format.hpp"
    "src/formatter/impl/detail/uint_fromchars_4digit.hpp"
    "src/formatter/impl/detail/valid_strftime_string.hpp"
)

set(src_logger
    "src/logger/log_macro_background.hpp"
    "src/logger/logger_background.hpp"
)

set(src_output
    "src/output/output.cpp"
    "src/output/output_cstream.cpp"
    "src/output/output_file.cpp"
    "src/output/output_interface.hpp"
    "src/output/output_interface_mixin.hpp"
    "src/output/output_locked_mixin.hpp"
    "src/output/output_null.cpp"
    "src/output/output_stream.cpp"
    "src/output/output_stream_mt.cpp"
)

set(src_output_impl
    "src/output/impl/out_cstream_mixin.hpp"
    "src/output/impl/out_file_mixin.hpp"
    "src/output/impl/out_file_posix.hpp"
    "src/output/impl/out_null_mixin.hpp"
    "src/output/impl/out_stream_mixin.hpp"
)

set(src_sink
    "src/sink/sink.cpp"
    "src/sink/sink_flush_time_mixin.hpp"
    "src/sink/sink_formatter_mixin.hpp"
    "src/sink/sink_interface.hpp"
    "src/sink/sink_interface_mixin.hpp"
    "src/sink/sink_msgblock_mixin.hpp"
    "src/sink/sink_null.cpp"
    "src/sink/sink_output_mixin.hpp"
    "src/sink/sink_sort.cpp"
    "src/sink/sink_unsort.cpp"
)

set(src_sink_impl
    "src/sink/impl/sink_null_mixin.hpp"
    "src/sink/impl/sink_sort_mixin.hpp"
    "src/sink/impl/sink_unsort_mixin.hpp"
)

set(FSTLOG_ALLOCATOR "${CMAKE_CURRENT_LIST_DIR}/include/fstlog/detail/pmr_allocator.hpp" CACHE STRING "Path to allocator.")
if(NOT EXISTS ${FSTLOG_ALLOCATOR})
	message(FATAL_ERROR "FSTLOG_ALLOCATOR path: ${FSTLOG_ALLOCATOR} does not exists!")
endif()
file(COPY_FILE 
	"${FSTLOG_ALLOCATOR}" 
	"${CMAKE_CURRENT_LIST_DIR}/include/fstlog/detail/fstlog_allocator.hpp"
	ONLY_IF_DIFFERENT)


set(ALL_FILES 
	${include_fstlog}
	${include_fstlog_detail}
	${include_fstlog_filter}
	${include_fstlog_formatter}
	${include_fstlog_logger}
	${include_fstlog_logger_detail}
	${include_fstlog_logger_detail_log}
	${include_fstlog_output}
	${include_fstlog_sink}
	${src}
	${src_detail}
	${src_detail_mixin}
	${src_filter}
	${src_formatter}
	${src_formatter_impl}
	${src_formatter_impl_detail}
	${src_logger}
	${src_output}
	${src_output_impl}
	${src_sink}
	${src_sink_impl}
)

source_group("include/fstlog" FILES ${include_fstlog})
source_group("include/fstlog/detail" FILES ${include_fstlog_detail})
source_group("include/fstlog/filter" FILES ${include_fstlog_filter})
source_group("include/fstlog/formatter" FILES ${include_fstlog_formatter})
source_group("include/fstlog/logger" FILES ${include_fstlog_logger})
source_group("include/fstlog/logger/detail" FILES ${include_fstlog_logger_detail})
source_group("include/fstlog/logger/detail/log" FILES ${include_fstlog_logger_detail_log})
source_group("include/fstlog/output" FILES ${include_fstlog_output})
source_group("include/fstlog/sink" FILES ${include_fstlog_sink})
source_group("src" FILES ${src})
source_group("src/detail" FILES ${src_detail})
source_group("src/detail/mixin" FILES ${src_detail_mixin})
source_group("src/filter" FILES ${src_filter})
source_group("src/formatter" FILES ${src_formatter})
source_group("src/formatter/impl" FILES ${src_formatter_impl})
source_group("src/formatter/impl/detail" FILES ${src_formatter_impl_detail})
source_group("src/logger" FILES ${src_logger})
source_group("src/output" FILES ${src_output})
source_group("src/output/impl" FILES ${src_output_impl})
source_group("src/sink" FILES ${src_sink})
source_group("src/sink/impl" FILES ${src_sink_impl})

add_library(fstlog ${ALL_FILES})
add_library(fstlog::fstlog ALIAS fstlog)
target_link_libraries(fstlog PUBLIC Threads::Threads)
set_target_properties(fstlog PROPERTIES VERSION ${FSTLOG_VERSION} SOVERSION  ${FSTLOG_VERSION_MAJOR}.${FSTLOG_VERSION_MINOR})

if(BUILD_SHARED_LIBS)
	target_compile_definitions(fstlog
		PUBLIC "FSTLOG_SHARED"
		PRIVATE "FSTLOG_EXPORT"
	)
endif()

if(MSVC AND NOT(FSTLOG_DEBUG))
	target_compile_definitions(fstlog PRIVATE
		FSTLOG_COMPILETIME_LOGLEVEL=Debug
	)
else()
		target_compile_definitions(fstlog 
			PRIVATE $<IF:$<CONFIG:Debug>,FSTLOG_COMPILETIME_LOGLEVEL=Trace,FSTLOG_COMPILETIME_LOGLEVEL=Debug>
			PUBLIC $<$<CONFIG:Debug>:FSTLOG_DEBUG>
		)
endif()

if(FSTLOG_DEFAULT_BUFFERSIZE)
	set(FSTLOG_DEFAULT_BUFFERSIZE ${FSTLOG_DEFAULT_BUFFERSIZE} CACHE STRING "Default log buffer size. (bytes, per thread)")
	target_compile_definitions(fstlog PRIVATE FSTLOG_DEFAULT_BUFFERSIZE=${FSTLOG_DEFAULT_BUFFERSIZE})
endif()
if(FSTLOG_MAX_BUFFERSIZE)
	set(FSTLOG_MAX_BUFFERSIZE ${FSTLOG_MAX_BUFFERSIZE} CACHE STRING "Max log buffer size. (bytes, per thread)")
	target_compile_definitions(fstlog PRIVATE FSTLOG_MAX_BUFFERSIZE=${FSTLOG_MAX_BUFFERSIZE})
endif()
if(FSTLOG_MIN_BUFFERSIZE)
	set(FSTLOG_MIN_BUFFERSIZE ${FSTLOG_MIN_BUFFERSIZE} CACHE STRING "Min log buffer size. (bytes, per thread)")
	target_compile_definitions(fstlog PRIVATE FSTLOG_MIN_BUFFERSIZE=${FSTLOG_MIN_BUFFERSIZE})
endif()
if(FSTLOG_MAX_MESSAGESIZE)
	set(FSTLOG_MAX_MESSAGESIZE ${FSTLOG_MAX_MESSAGESIZE} CACHE STRING "Max log message size (bytes, per thread)")
	target_compile_definitions(fstlog PRIVATE FSTLOG_MAX_MESSAGESIZE=${FSTLOG_MAX_MESSAGESIZE})
endif()
option(FSTLOG_NO_NESTING "Disable logging nested containers. Only used by user code and passed to compiler of user code." OFF)
if(FSTLOG_NO_NESTING)
	target_compile_definitions(fstlog PUBLIC FSTLOG_NO_NESTING)
endif()
if(FSTLOG_CONTAINER_NESTING_DEPTH)
	set(FSTLOG_CONTAINER_NESTING_DEPTH ${FSTLOG_CONTAINER_NESTING_DEPTH} CACHE STRING "Nesting depth of logged containers.")
	target_compile_definitions(fstlog PRIVATE FSTLOG_CONTAINER_NESTING_DEPTH=${FSTLOG_CONTAINER_NESTING_DEPTH})
endif()
if(FSTLOG_POLLINTERVAL)
	set(FSTLOG_POLLINTERVAL ${FSTLOG_POLLINTERVAL} CACHE STRING "Default interval of log buffer flushing (in milliseconds).")
	target_compile_definitions(fstlog PRIVATE FSTLOG_POLLINTERVAL=${FSTLOG_POLLINTERVAL})
endif()
if(FSTLOG_SINK_FLUSHINTERVAL)
	set(FSTLOG_SINK_FLUSHINTERVAL ${FSTLOG_SINK_FLUSHINTERVAL} CACHE STRING "Default interval of sink flushing (in milliseconds).")
	target_compile_definitions(fstlog PRIVATE FSTLOG_SINK_FLUSHINTERVAL=${FSTLOG_SINK_FLUSHINTERVAL})
endif()

#--------------------
# Include directories
#--------------------
target_include_directories(fstlog
	PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
	PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>"
	PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

#--------
# Install
#--------
install(
	TARGETS fstlog
	EXPORT fstlogTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(FILES ${include_fstlog} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fstlog")
install(FILES ${include_fstlog_detail} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fstlog/detail")
install(FILES ${include_fstlog_filter} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fstlog/filter")
install(FILES ${include_fstlog_formatter} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fstlog/formatter")
install(FILES ${include_fstlog_logger} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fstlog/logger")
install(FILES ${include_fstlog_logger_detail} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fstlog/logger/detail")
install(FILES ${include_fstlog_logger_detail_log} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fstlog/logger/detail/log")
install(FILES ${include_fstlog_output} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fstlog/output")
install(FILES ${include_fstlog_sink} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fstlog/sink")

install(EXPORT fstlogTargets
        FILE fstlogTargets.cmake
        NAMESPACE fstlog::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fstlog
)

# ---------------------
# Package Configuration
# ---------------------
include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/fstlogConfig.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/fstlogConfig.cmake"
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fstlog
)

set_property(
	TARGET fstlog PROPERTY
	INTERFACE_fstlog_MAJOR_VERSION ${FSTLOG_VERSION_MAJOR}
)
set_property(
	TARGET fstlog APPEND PROPERTY
	COMPATIBLE_INTERFACE_STRING fstog_MAJOR_VERSION
)

# generate the version file for the config file
if(FSTLOG_VERSION_MAJOR EQUAL 0)
	write_basic_package_version_file(
	  "${CMAKE_CURRENT_BINARY_DIR}/fstlogConfigVersion.cmake"
	  VERSION "${FSTLOG_VERSION}"
	  COMPATIBILITY ExactVersion
	)
else()
	write_basic_package_version_file(
	  "${CMAKE_CURRENT_BINARY_DIR}/fstlogConfigVersion.cmake"
	  VERSION "${FSTLOG_VERSION}"
	  COMPATIBILITY SameMajorVersion
	)
endif()


install(FILES
          "${CMAKE_CURRENT_BINARY_DIR}/fstlogConfig.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/fstlogConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fstlog
)


if(CMAKE_BUILD_TYPE)
# ----------
# pkg-config
# ----------
	set(PKG_LIBNAME "fstlog")
	string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE_UPPER)
	set(PKG_DEFINES "")
	if (BUILD_TYPE_UPPER STREQUAL "DEBUG")
		set(PKG_LIBNAME "${PKG_LIBNAME}${CMAKE_DEBUG_POSTFIX}")
		set(PKG_DEFINES "-DFSTLOG_DEBUG")
	endif ()
	if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
			set(PKG_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}")
	else()
		set(PKG_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")
	endif()
	if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
		set(PKG_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
	else()
		set(PKG_LIBDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
	endif()

	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/cmake/libfstlog.pc.in"
		"${CMAKE_CURRENT_BINARY_DIR}/lib${PKG_LIBNAME}.pc" 
		@ONLY)
	install(FILES
		"${CMAKE_CURRENT_BINARY_DIR}/lib${PKG_LIBNAME}.pc"
		DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
	)
endif()

# -------------------------------------
# Exporting Targets from the Build Tree
# -------------------------------------
export(EXPORT fstlogTargets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/fstlogTargets.cmake"
       NAMESPACE fstlog::
)
