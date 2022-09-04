cmake_minimum_required(VERSION 3.16.6)
project(libevent_cpp)

set(CMAKE_CXX_STANDARD 11)

include_directories(src/)


aux_source_directory(src/base BASE_SRC)
aux_source_directory(src/event EVENT_SRC)

add_executable(libevent_cpp ${BASE_SRC} ${EVENT_SRC})

add_subdirectory(test)
