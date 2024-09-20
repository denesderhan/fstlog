#Copyright © 2022, Dénes Derhán.
#Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).

find_package(Threads REQUIRED)

set(src
    "multi_threaded.cpp"
)
source_group("Source Files" FILES ${src})

set(ALL_FILES
    ${src}
)

add_executable(multi_threaded ${ALL_FILES})
set_target_properties(multi_threaded PROPERTIES 
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	FOLDER "examples"
)

################################################################################
# Dependencies
################################################################################

target_link_libraries(multi_threaded
	PRIVATE fstlog
)
