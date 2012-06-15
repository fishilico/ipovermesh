# Run cmake . to build Makefile

cmake_minimum_required(VERSION 2.6)

project(IPoverMESH)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Wno-unused-parameter")

# Use Boost
find_package(Boost 1.40.0 COMPONENTS thread)
#find_package(Boost 1.40.0 COMPONENTS regex signals FATAL_ERROR)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

file(GLOB_RECURSE sources src/core/*.cpp src/mesh/*.cpp src/net/*.cpp src/parser/*.cpp)

add_executable(test ${sources} src/test.cpp)
target_link_libraries(test ${Boost_LIBRARIES})

