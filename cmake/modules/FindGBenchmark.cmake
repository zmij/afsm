# Findbenchmark.cmake
# - Try to find benchmark
#
# The following variables are optionally searched for defaults
#  GBENCH_ROOT_DIR:  Base directory where all benchmark components are found
#
# Once done this will define
#  GBENCH_FOUND - System has benchmark
#  GBENCH_INCLUDE_DIRS - The benchmark include directories
#  GBENCH_LIBRARIES - The libraries needed to use benchmark

set(GBENCH_ROOT_DIR "" CACHE PATH "Folder containing benchmark")

find_path(GBENCH_INCLUDE_DIR "benchmark/benchmark.h"
  PATHS ${GBENCH_ROOT_DIR}
  PATH_SUFFIXES include
  NO_DEFAULT_PATH)
find_path(GBENCH_INCLUDE_DIR "benchmark/benchmark.h")

find_library(GBENCH_LIBRARY NAMES "benchmark"
  PATHS ${GBENCH_ROOT_DIR}
  PATH_SUFFIXES lib lib64
  NO_DEFAULT_PATH)
find_library(GBENCH_LIBRARY NAMES "benchmark")

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set benchmark_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(GBENCH FOUND_VAR GBENCH_FOUND
  REQUIRED_VARS GBENCH_LIBRARY
  GBENCH_INCLUDE_DIR)

if(GBENCH_FOUND)
  set(GBENCH_LIBRARIES ${GBENCH_LIBRARY})
  set(GBENCH_INCLUDE_DIRS ${GBENCH_INCLUDE_DIR})
endif()

mark_as_advanced(GBENCH_INCLUDE_DIR GBENCH_LIBRARY)
