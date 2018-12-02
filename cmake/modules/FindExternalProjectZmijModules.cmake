#FindExternalProjectZmijModules.cmake
# Created on: Dec 2, 2018
#     Author: ser-fedorov

if (TARGET libzmijcmake)
    return()
endif()

find_program(GIT_EXECUTABLE git)

if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/cmake-scripts)
    execute_process(COMMAND ${GIT_EXECUTABLE} pull
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cmake-scripts)
else()
    execute_process(COMMAND ${GIT_EXECUTABLE} clone -b develop https://github.com/zmij/cmake-scripts.git cmake-scripts
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

set(ZMIJ_CMAKE_SCRIPTS ${CMAKE_CURRENT_BINARY_DIR}/cmake-scripts)
add_library(libzmijcmake IMPORTED INTERFACE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${ZMIJ_CMAKE_SCRIPTS}/modules")
