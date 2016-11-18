#    /afsm/benchmark/CMakeLists.txt
#
#    @author zmij
#    @date May 26, 2016

cmake_minimum_required(VERSION 2.6)

enable_testing()

if (NOT GBENCH_FOUND)
    find_package(GBenchmark REQUIRED)
endif()
if (NOT CMAKE_THREAD_LIBS_INIT)
    find_package(Threads REQUIRED)
endif()

# Add your package dependencies for test here

include_directories(${GBENCH_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../examples)

set(benchmark_SRCS vending_benchmark.cpp)
add_executable(benchmark-afsm ${benchmark_SRCS})
target_link_libraries(benchmark-afsm
    ${GBENCH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT})

add_test(
    NAME benchmark-afsm
    COMMAND benchmark-afsm
)

# Benchmark MSM
find_package(Boost 1.58)
if (Boost_FOUND)
    message(STATUS "Making benchmark with boost::msm library")
    include_directories(${Boost_INCLUDE_DIRS})
    set(benchmark_msm_SRCS vending_msm_benchmark.cpp)
    add_executable(benchmark-msm ${benchmark_msm_SRCS})
    target_link_libraries(benchmark-msm
        ${GBENCH_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT})

    add_test(
        NAME benchmark-msm
        COMMAND benchmark-msm
    )
endif()
