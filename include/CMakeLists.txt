#    /afsm/include/CMakeLists.txt
#
#    @author zmij
#    @date Nov 18, 2016

cmake_minimum_required(VERSION 2.6)

install(
    DIRECTORY afsm
    DESTINATION include
    FILES_MATCHING PATTERN *.hpp PATTERN *.inl
)
