cmake_minimum_required(VERSION 3.5)

project(myproject)

set(CMAKE_CXX_STANDARD 17)


set(SRC_FILES
    src/blifoutput.cpp
    src/blifoutput.hpp
    src/SR.cpp
    src/SR.hpp
    src/main.cpp
)


add_executable(main ${SRC_FILES})