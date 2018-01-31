# Specify the minimum version for CMake

cmake_minimum_required(VERSION 2.8)

# Project's name

project(boglfw)
# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/dist/lib)

# The following folder will be included
include_directories("${PROJECT_SOURCE_DIR}/include")

file(GLOB_RECURSE sources	include/*.h)
file(GLOB_RECURSE sources 	src/*.cpp)
file(GLOB_RECURSE data		data/*.*)

list(REMOVE_ITEM sources ${CMAKE_SOURCE_DIR}/src/main.cpp)

add_library(boglfw ${sources} ${data})

set_property(TARGET boglfw PROPERTY CXX_STANDARD 14)
target_compile_options(boglfw PUBLIC -Wall -DGLM_ENABLE_EXPERIMENTAL -DGLM_FORCE_RADIANS -std=c++14)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

ADD_CUSTOM_COMMAND(
    TARGET boglfw
    POST_BUILD
    COMMAND cp -r ${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/dist/
    COMMAND cp -r ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/dist/
    COMMAND cp ${CMAKE_SOURCE_DIR}/src/main.cpp ${CMAKE_BINARY_DIR}/dist/
)

