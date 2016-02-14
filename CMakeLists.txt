cmake_minimum_required(VERSION 3.3)
project(coroutines_example)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES main.cpp)

find_package( Boost 1.59 REQUIRED COMPONENTS coroutine system context)
include_directories(${Boost_INCLUDE_DIR})

add_executable(coroutines_example ${SOURCE_FILES})

message("${Boost_LIBRARIES}")

target_link_libraries(coroutines_example ${Boost_LIBRARIES})
