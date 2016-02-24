cmake_minimum_required (VERSION 2.8.11)
project (csg-convert)

include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/include")

add_definitions(-std=c++11)

add_executable (csg-convert src/main.cpp)