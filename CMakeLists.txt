cmake_minimum_required(VERSION 3.16.3)
project(libevent_cpp)

set(CMAKE_CXX_STANDARD 11)

include_directories(
    src
)

file(GLOB_RECURSE SRCS
    src/*.cpp
)

add_library(event_cpp SHARED ${SRCS})

add_subdirectory(test)
