#    /afsm/examples/CMakeLists.txt
#
#    @author zmij
#    @date Nov 14, 2016

cmake_minimum_required(VERSION 2.6)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/ansi-colors/include
)

file(GLOB examples *.cpp)
foreach(file ${examples})
    get_filename_component(tgt_name ${file} NAME_WE)
    add_executable(${tgt_name} ${file}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/ansi-colors/src/ansi_colors.cpp)
endforeach()
