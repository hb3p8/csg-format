cmake_minimum_required (VERSION 2.8.11)
project (csgparser)

include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/include")

add_definitions(-std=c++11)

add_subdirectory("${PROJECT_SOURCE_DIR}/src/json11")

# enable_testing()

# csgparser lib
set(csgparser_SRCS src/csgparser.cpp src/csgparser.hpp)

add_library(csgparser STATIC ${csgparser_SRCS})
target_link_libraries (csgparser json11)

# tests
# add_test(csgparser_test csgparser_test)
# add_executable(csgparser_test ${json11_SRCS} src/tests/test.cpp)

# csg2json exe
add_executable (csg2json src/csg2json.cpp)
target_link_libraries (csg2json csgparser)

