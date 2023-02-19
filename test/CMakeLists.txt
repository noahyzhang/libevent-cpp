cmake_minimum_required(VERSION 3.16)

project(libevent_cpp_test)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")

include_directories(
    ../include
)

# 测试 IO_EVENT 事件
file(GLOB IO_EVENT_TEST_SRC
    sample/io_event_test.cpp
)
add_executable(io_event_test ${IO_EVENT_TEST_SRC})
target_link_libraries(io_event_test 
    event_cpp
)

# 测试 SIGNAL_EVENT 事件
file(GLOB SIGNAL_EVENT_TEST_SRC
    sample/signal_event_test.cpp
)
add_executable(signal_event_test ${SIGNAL_EVENT_TEST_SRC})
target_link_libraries(signal_event_test
    event_cpp
)

# 测试 TIME_EVENT 事件
file(GLOB TIME_EVENT_TEST_SRC
    sample/time_event_test.cpp
)
add_executable(time_event_test ${TIME_EVENT_TEST_SRC})
target_link_libraries(time_event_test
    event_cpp
)

