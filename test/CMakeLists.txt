cmake_minimum_required(VERSION 3.16)

project(libevent_cpp_test)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")

include_directories(/data/home/noahyzhang/cplus_project/libevent-cpp/src/)

file(GLOB_RECURSE SRC /data/home/noahyzhang/cplus_project/libevent-cpp/src/*.cpp)

# aux_source_directory(/data/home/noahyzhang/cplus_project/libevent-cpp/src/ SRC)
# aux_source_directory(/data/home/noahyzhang/cplus_project/libevent-cpp/src/event EVENT_SRC)
# aux_source_directory(/data/home/noahyzhang/cplus_project/libevent-cpp/src/util/log UTIL_LOG_SRC)

# add_executable(libevent_cpp_test ${BASE_SRC} ${EVENT_SRC} ${UTIL_LOG_SRC} sample/io_event_test.cpp)
add_executable(libevent_cpp_test ${SRC} sample/io_event_test.cpp)

