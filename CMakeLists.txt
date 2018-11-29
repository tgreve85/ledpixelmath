cmake_minimum_required(VERSION 3.9)
project(ledpixelmath)

set(CMAKE_CXX_STANDARD 11)

include_directories(/usr/include/python3.5 /usr/include/python3.6 /usr/include/python3.7)

add_library(ledpixelmath SHARED
        ledpixelmath.cpp
        )

target_link_libraries(ledpixelmath)