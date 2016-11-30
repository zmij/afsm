#    meta/cmake/CMakeLists.txt
#
#    Try to find a directory where to put FindMetaPushkin script
#
#    @author zmij
#    @date Nov 18, 2016

cmake_minimum_required(VERSION 2.6)

if(NOT CMAKE_SCRIPT_INSTALL_DIR)
    find_path(CMAKE_SCRIPT_INSTALL_DIR "CheckLanguage.cmake"
        PATHS ${CMAKE_ROOT}
        PATH_SUFFIXES Modules )
endif()

if (CMAKE_SCRIPT_INSTALL_DIR)
    file(GLOB find_scripts Find*.cmake)
    install(
        FILES ${find_scripts}
        DESTINATION ${CMAKE_SCRIPT_INSTALL_DIR})
else()
    message(WARNING "Failed to find target directory for installing cmake Find scripts")
endif()
