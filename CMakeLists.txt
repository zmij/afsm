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

option(BUILD_TESTS "Build test programs" ON)
option(BUILD_BENCHMARKS "Build benchmarks" ON)
option(BUILD_EXAMPLES "Build example programs" OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

add_definitions("-std=c++11")
add_definitions(-Wall -Werror -Wextra -pedantic -Weffc++
    -Wno-non-virtual-dtor # I really know what I am exactly doing
)

set(${LIB_NAME}_LIB ${lib_name})

# Add subdirectories here

if (NOT METAPUSHKIN_FOUND)
    add_subdirectory(lib/meta)
endif()

message(STATUS "Metapushkin include dir ${METAPUSHKIN_INCLUDE_DIRS}")
include_directories(${METAPUSHKIN_INCLUDE_DIRS})

add_subdirectory(include)
add_subdirectory(cmake)

if (BUILD_TESTS)
    enable_testing()
    if (NOT PSST_ANSI_COLORS_INCLUDE_DIR)
    add_subdirectory(lib/ansi-colors)
    endif()

    include_directories(${PSST_ANSI_COLORS_INCLUDE_DIR})
    link_directories(${PSST_ANSI_COLORS_LINK_DIR})
    add_subdirectory(test)
endif()
if (BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif()

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

get_directory_property(has_parent PARENT_DIRECTORY)
if (has_parent)
    set(${LIB_NAME}_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${METAPUSHKIN_INCLUDE_DIRS}
        CACHE INTERNAL "Path to afsm libaray includes" )
endif()
