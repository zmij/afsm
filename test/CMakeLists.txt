#    /afsm/test/CMakeLists.txt
#
#    @author zmij
#    @date May 26, 2016

cmake_minimum_required(VERSION 2.6)
if (NOT TARGET test-afsm-base)
if (NOT CMAKE_THREAD_LIBS_INIT)
    find_package(Threads REQUIRED)
endif()

# Add your package dependencies for test here

include_directories(${GTEST_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../examples
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/ansi-colors/include)

set(
    test_program_SRCS
    # Add your sources here
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/ansi-colors/src/ansi_colors.cpp
    static_tests.cpp
    state_machine_with_internal_transitions.cpp
    fsm_parts_test.cpp
    orthogonal_states_test.cpp
    transaction_fsm.cpp
    transaction_priority_fsm.cpp
    common_base_test.cpp
    vending_machine_test.cpp
    pushdown_tests.cpp
)
add_executable(test-afsm-base ${test_program_SRCS})
target_link_libraries(
    test-afsm-base
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
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
endif()
