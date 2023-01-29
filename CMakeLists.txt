cmake_minimum_required(VERSION 3.16.3)
project(libevent_cpp)

set(CMAKE_CXX_STANDARD 11)

include_directories(
    src
)

file(GLOB SRCS 
    src/base/*.cpp
    src/event/*.cpp
    src/util/*.cpp
)

add_library(event_cpp SHARED ${SRCS})

add_subdirectory(test)
