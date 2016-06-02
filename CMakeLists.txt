#    CMakeLists.txt for ansi-colors library
#
#    @author zmij
#    @date Nov 30, 2015

cmake_minimum_required(VERSION 2.6)

# Set library name here
set(lib_name ansi-colors)
string(TOUPPER ${lib_name} LIB_NAME)

if (NOT PROJECT_PREFIX)
    set(PROJECT_PREFIX psst)
endif()
set(_pname ${PROJECT_PREFIX}-${lib_name})
if (PROJECT_VERSION)
    set(_pversion ${PROJECT_VERSION})
else()
    set(_pversion 0.1.0)
endif()

if (${CMAKE_VERSION} VERSION_GREATER "3.0")
    cmake_policy(SET CMP0048 NEW)
    project(${_pname} VERSION ${_pversion})
else()
    project(${_pname})
    set(PROJECT_VERSION ${_pversion})
endif()

option(BUILD_EXAMPLE "Build example application" OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_definitions("-std=c++11")

set(PSST_${LIB_NAME}_LIB ${PROJECT_PREFIX}-${lib_name})

# Add subdirectories here
add_subdirectory(src)
if(BUILD_EXAMPLE)
add_subdirectory(example)
endif()

get_directory_property(has_parent PARENT_DIRECTORY)
if (has_parent)
    set(PSST_${LIB_NAME}_LIB ${PROJECT_PREFIX}-${lib_name} CACHE INTERNAL "Name of ansi-colors library target")
    set(PSST_${LIB_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "Path to ansi-colors library includes" )
endif()
