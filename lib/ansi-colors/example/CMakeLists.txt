#    /ansi-colors/example/CMakeLists.txt
#
#    @author zmij
#    @date Jun 2, 2016

cmake_minimum_required(VERSION 2.6)

add_executable(psst-ansi-colors-example ansi_colors_example.cpp)
target_link_libraries(psst-ansi-colors-example ${PSST_${LIB_NAME}_LIB})
