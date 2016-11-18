# FindMetaPushkin.cmake
# Try to find metapuskin library
#
# The following variables are optionally searched for defaults
#  METAPUSHKIN_ROOT_DIR:  Base directory where include directory can be found
#
# Once done this will define
#  METAPUSHKIN_FOUND    - System has metapushkin
#  METAPUSHKIN_INCLUDE_DIRS - The metapushkin include directories

if(NOT METAPUSHKIN_FOUND)

set(METAPUSHKIN_ROOT_DIR "" CACHE PATH "Folder containing metapushkin")

find_path(METAPUSHKIN_INCLUDE_DIR "pushkin/meta.hpp"
    PATHS ${METAPUSHKIN_ROOT_DIR}
    PATH_SUFFIXES include
    NO_DEFAULT_PATH)

find_path(METAPUSHKIN_INCLUDE_DIR "pushkin/meta.hpp")

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set benchmark_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(METAPUSHKIN FOUND_VAR METAPUSHKIN_FOUND
    REQUIRED_VARS METAPUSHKIN_INCLUDE_DIR)

if(METAPUSHKIN_FOUND)
    set(METAPUSHKIN_INCLUDE_DIRS ${METAPUSHKIN_INCLUDE_DIR})
endif()

mark_as_advanced(METAPUSHKIN_INCLUDE_DIR)

endif()
