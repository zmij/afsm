# /afsm/cmake/FindAFSM.cmake
# Try to find afsm library
#
# The following variables are optionally searched for defaults
#  AFSM_ROOT_DIR:  Base directory where include directory can be found
#
# Once done this will define
#  AFSM_FOUND    - System has afsm library
#  AFSM_INCLUDE_DIRS - The afsm include directories

# Library requires metapushkin library and will additionally search for it

find_package(MetaPushkin REQUIRED)

if(NOT AFSM_FOUND)

set(AFSM_ROOT_DIR "" CACHE PATH "Folder containing afsm library")

find_path(AFSM_INCLUDE_DIR "afsm/fsm.hpp"
    PATHS ${AFSM_ROOT_DIR}
    PATH_SUFFIXES include
    NO_DEFAULT_PATH)

find_path(AFSM_INCLUDE_DIR "afsm/fsm.hpp")

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set benchmark_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(AFSM FOUND_VAR AFSM_FOUND
    REQUIRED_VARS AFSM_INCLUDE_DIR)

if(AFSM_FOUND)
    set(AFSM_INCLUDE_DIRS ${AFSM_INCLUDE_DIR})
endif()

mark_as_advanced(AFSM_INCLUDE_DIR)

endif()
