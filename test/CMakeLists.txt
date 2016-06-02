#    /afsm/test/CMakeLists.txt
#
#    @author zmij
#    @date May 26, 2016

cmake_minimum_required(VERSION 2.6)

if (NOT GTEST_INCLUDE_DIRS)
    find_package(GTest REQUIRED)
endif()
if (NOT CMAKE_THREAD_LIBS_INIT)
    find_package(Threads REQUIRED)
endif()

# Add your package dependencies for test here

include_directories(${GTEST_INCLUDE_DIRS})

set(
    test_program_SRCS
    # Add your sources here
    static_tests.cpp
    fsm_parts_test.cpp
    transaction_fsm.cpp
    common_base_test.cpp
)
add_executable(test-afsm-base ${test_program_SRCS})
target_link_libraries(
    test-afsm-base
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    # Add your libraries here
    ${PSST_ANSI_COLORS_LIB}
)

if (GTEST_XML_OUTPUT)
    set (
        TEST_ARGS
        --gtest_output=xml:test-utils-detail.xml
    )
endif()

add_test(
    NAME test-afsm-base
    COMMAND test-afsm-base ${TEST_ARGS}
)
