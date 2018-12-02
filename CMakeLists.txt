#    CMakeLists.txt for afsm library
#
#    @author zmij
#    @date Nov 30, 2015

cmake_minimum_required(VERSION 2.6)

# Set library name here
set(lib_name afsm)
string(TOUPPER ${lib_name} LIB_NAME)

if (PROJECT_VERSION)
    set(_pversion ${PROJECT_VERSION})
else()
    set(_pversion 0.1.0)
endif()

if (${CMAKE_VERSION} VERSION_GREATER "3.0")
    cmake_policy(SET CMP0048 NEW)
    project(${lib_name} VERSION ${_pversion})
else()
    project(${lib_name})
    set(PROJECT_VERSION ${_pversion})
endif()

option(AFSM_BUILD_TESTS "Build test programs" ON)
option(AFSM_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(AFSM_BUILD_EXAMPLES "Build example programs" OFF)

option(USE_CCACHE "Use ccache for build" ON)
if (USE_CCACHE)
    find_program(CCACHE ccache)
    if (CCACHE)
        message(STATUS "ccache found and enabled")
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
    else()
        message(WARNING "ccache enabled, but not found")
    endif()
else()
    message(STATUS "ccache disabled")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif()
message(STATUS "AFSM C++ standard ${CMAKE_CXX_STANDARD}")
add_definitions(-Wall -Werror -Wextra -pedantic -Weffc++
    -Wno-non-virtual-dtor # I really know what I am exactly doing
)

set(${LIB_NAME}_LIB ${lib_name})

find_package(ExternalProjectZmijModules)
find_package(ExternalProjectMetapushkin)

message(STATUS "Metapushkin include dir ${METAPUSHKIN_INCLUDE_DIRS}")
include_directories(${METAPUSHKIN_INCLUDE_DIRS})

add_subdirectory(include)
add_subdirectory(cmake)

add_library(afsm INTERFACE)
add_dependencies(afsm libmetapushkin)

if (AFSM_BUILD_TESTS OR AFSM_BUILD_BENCHMARKS)
    find_package(ExternalProjectGTest)
endif()

if (AFSM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
if (AFSM_BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif()

if (AFSM_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

get_directory_property(has_parent PARENT_DIRECTORY)
if (has_parent)
    set(${LIB_NAME}_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${METAPUSHKIN_INCLUDE_DIRS}
        CACHE INTERNAL "Path to afsm libaray includes" )
endif()
