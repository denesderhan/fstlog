#Copyright © 2022, Dénes Derhán.
#Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).

find_package(Threads REQUIRED)

set(src
    "hello_world.cpp"
)
source_group("Source Files" FILES ${src})

set(ALL_FILES
    ${src}
)

add_executable(hello_world ${ALL_FILES})
set_target_properties(hello_world PROPERTIES 
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	FOLDER "examples"
)

################################################################################
# Dependencies
################################################################################

target_link_libraries(hello_world
	PRIVATE fstlog
)
