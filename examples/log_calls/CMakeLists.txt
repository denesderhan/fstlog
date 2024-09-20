#Copyright © 2022, Dénes Derhán.
#Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).

find_package(Threads REQUIRED)

set(src
    "log_calls.cpp"
)
source_group("Source Files" FILES ${src})

set(ALL_FILES
    ${src}
)

add_executable(log_calls ${ALL_FILES})
set_target_properties(log_calls PROPERTIES 
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	FOLDER "examples"
)

################################################################################
# Dependencies
################################################################################

target_link_libraries(log_calls
	PRIVATE fstlog
)
