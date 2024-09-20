#Copyright © 2022, Dénes Derhán.
#Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).

# Get version from fstlog/version.h and put it in FSTLOG_VERSION
function(get_version)
	file(READ "${CMAKE_CURRENT_LIST_DIR}/fstlog/include/fstlog/version.hpp" version_hpp)
	
    string(REGEX MATCH "FSTLOG_VERSION_MAJOR ([0-9]+)" _ "${version_hpp}")
    if(NOT CMAKE_MATCH_COUNT EQUAL 1)
        message(FATAL_ERROR "Could not extract version.")
    endif()
    set(v_major ${CMAKE_MATCH_1})

    string(REGEX MATCH "FSTLOG_VERSION_MINOR ([0-9]+)" _ "${version_hpp}")
    if(NOT CMAKE_MATCH_COUNT EQUAL 1)
        message(FATAL_ERROR "Could not extract version.")
    endif()
    set(v_minor ${CMAKE_MATCH_1})

    string(REGEX MATCH "FSTLOG_VERSION_PATCH ([0-9]+)" _ "${version_hpp}")
    if(NOT CMAKE_MATCH_COUNT EQUAL 1)
        message(FATAL_ERROR "Could not extract version.")
    endif()
    set(v_patch ${CMAKE_MATCH_1})

	set(FSTLOG_VERSION_MAJOR "${v_major}"  PARENT_SCOPE)
	set(FSTLOG_VERSION_MINOR "${v_minor}"  PARENT_SCOPE)
	set(FSTLOG_VERSION_PATCH "${v_patch}"  PARENT_SCOPE)
    set(FSTLOG_VERSION "${v_major}.${v_minor}.${v_patch}"  PARENT_SCOPE)
endfunction()
