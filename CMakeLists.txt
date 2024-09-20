#Copyright © 2022, Dénes Derhán.
#Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).

cmake_minimum_required(VERSION 3.14)
cmake_policy(SET CMP0117 NEW) # MSVC do not use /GR by default

#---------
# OPTIONS
#---------

option(FSTLOG_TESTS "Generate tests project." OFF)
option(FSTLOG_EXAMPLES "Generate examples project." ON)
option(BUILD_SHARED_LIBS "Build fstlog as shared library." OFF)
option(CMAKE_EXPORT_COMPILE_COMMANDS "Export compile commands." ON)
set(CMAKE_DEBUG_POSTFIX d CACHE STRING "Debug filename postfix.")
option(FSTLOG_DEBUG "MSVC setting for Debug config (Non optimized build for Debug.)" OFF)
mark_as_advanced(FORCE FSTLOG_DEBUG CMAKE_DEBUG_POSTFIX CMAKE_EXPORT_COMPILE_COMMANDS)

#--------------------
# shared lib/tests check
#--------------------
if (BUILD_SHARED_LIBS AND FSTLOG_TESTS)
		message( FATAL_ERROR "Can not build tests project with shared library. (set -DFSTLOG_TESTS=OFF)" )
endif()

#----------------------
# C++ standard setting
#----------------------
set(CMAKE_CXX_STANDARD 20 CACHE STRING "C++ standard version")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if( CMAKE_CXX_STANDARD VERSION_LESS 17 )
  message( FATAL_ERROR "Minimum C++17 required!" )
else()
	message(STATUS "C++ Standard = ${CMAKE_CXX_STANDARD}")
endif()

#----------------------------------------
# Hiding non API symbols from shared lib
#----------------------------------------
if(BUILD_SHARED_LIBS)
	set(CMAKE_CXX_VISIBILITY_PRESET hidden CACHE STRING "Default visibility for shared lib symbols.")
	if("${CMAKE_CXX_VISIBILITY_PRESET}" STREQUAL "hidden")
		set(CMAKE_VISIBILITY_INLINES_HIDDEN YES CACHE STRING "Inlines hidden in shared lib.")
	endif()
endif()

#-----------------
# extract version
#-----------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(get_version)
get_version()

#----------------
# fstlog project
#----------------
project(fstlog VERSION ${FSTLOG_VERSION} LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#------------
# Build type
#------------
get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(is_multi_config)
	if (NOT CMAKE_CONFIGURATION_TYPES)
		set(CMAKE_CONFIGURATION_TYPES Release;Debug CACHE STRING "Multi-config configurations." FORCE)
	endif()
	set(BUILD_TYPE_MSG "Multi-config (${CMAKE_CONFIGURATION_TYPES})")
	message(STATUS "Genearator: ${CMAKE_GENERATOR} (multi-config).")
else()
	if (NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type, Release/Debug/..." FORCE)
	endif()
	message(STATUS "Genearator: ${CMAKE_GENERATOR} (single-config).") 
	set(BUILD_TYPE_MSG ${CMAKE_BUILD_TYPE})
endif()

#----------------------
# Print project info
#----------------------
set(STAT_OR_SHARE Static)
if(BUILD_SHARED_LIBS)
	set(STAT_OR_SHARE Shared)
endif()
message(STATUS 	"PROJECT ${CMAKE_PROJECT_NAME} ${CMAKE_PROJECT_VERSION} ${STAT_OR_SHARE} ${BUILD_TYPE_MSG}")

#-----------------
# Output dirs 
#-----------------
include(GNUInstallDirs)
if((MSVC OR is_multi_config) AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE "${CMAKE_CURRENT_BINARY_DIR}/bin/${STAT_OR_SHARE}/fstlog")
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/${STAT_OR_SHARE} CACHE STRING "")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/${STAT_OR_SHARE} CACHE STRING "")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/${STAT_OR_SHARE} CACHE STRING "")
set(CMAKE_PDB_OUTPUT_DIRECTORY     ${CMAKE_CURRENT_BINARY_DIR}/bin/${STAT_OR_SHARE} CACHE STRING "")

#--------------------------------
# compile options; win Debug fix
#--------------------------------
if(MSVC)
	add_compile_options(
		/options:strict			#abort on unrecognized compiler option
		/permissive-			#CXX_EXTENSIONS=OFF does not set /permissive-
		/Zc:__cplusplus			#set c++ standard feature test
		/MP						#multi process compilation
		/wd4324					#supress automatic padding warning
		/Zc:tlsGuards-			#Do not use TLS initialization checks. (Remove if causes errors for dynamically loaded dll-s.) see: https://developercommunity.visualstudio.com/t/5x-performance-degrade-in-vs2019-1671-for-thread-l/1184366 https://developercommunity.visualstudio.com/t/thread-local-variables-fail-to-be-initialized-when/124121
		/utf-8					#UTF8 for source and execution char sets
		/GR-					#disable RTTI
	)
	add_compile_definitions(
		"_UNICODE"				#set unicode for C runtime headers
		"UNICODE"				#win API calls will map to unicode versions
	)
	if(NOT FSTLOG_DEBUG)
		add_compile_options(
			/GS-				#disable buffer security checks
			/sdl-				#disable additional security checks
		)
		if(is_multi_config OR (CMAKE_BUILD_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$"))
			message(STATUS "Configuring optimized Debug, to debug fstlog, use -DFSTLOG_DEBUG=ON.")
		endif()
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_RELEASE}")
		set(CMAKE_C_FLAGS_FLAGS_DEBUG "${CMAKE_C_FLAGS_RELEASE}" CACHE STRING "Flags used by the C compiler during DEBUG builds." FORCE)
		set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_RELEASE}" CACHE STRING "Flags used by the linker during DEBUG builds." FORCE)
		set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}" CACHE STRING "Flags used by the linker during the creation of shared libraries during DEBUG builds." FORCE)
		set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_RELEASE}" CACHE STRING "Flags used by the linker during the creation of modules during DEBUG builds." FORCE)
		set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_RELEASE}" CACHE STRING "Flags used by the linker during the creation of static libraries during DEBUG builds." FORCE)
		set(CMAKE_RC_FLAGS_DEBUG "${CMAKE_RC_FLAGS_RELEASE}" CACHE STRING "Flags for Windows Resource Compiler during DEBUG builds." FORCE)
		STRING( REPLACE "/MD" "/MDd" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} )
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING "Flags used by the CXX compiler during DEBUG builds." FORCE)
	else()
		if(is_multi_config OR (CMAKE_BUILD_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$"))
			message(STATUS "Configuring Debug to debug fstlog, to make optimized Debug, use -DFSTLOG_DEBUG=OFF")
		endif()
	endif()
else()
	add_compile_options(
		-fno-rtti
	)
endif()

#----------------------
#fstlog library target
#----------------------
message(STATUS "Generating fstlog library.")
add_subdirectory(fstlog)

#----------------------
# tests target
#----------------------
if (FSTLOG_TESTS)
	message(STATUS "Generating tests.")
    add_subdirectory(tests)
endif()

#----------------------
# examples target
#----------------------
if (FSTLOG_EXAMPLES)
	message(STATUS "Generating examples.")
	add_subdirectory(examples)
endif()
