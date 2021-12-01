cmake_minimum_required(VERSION 3.0.0)
project(sha256 VERSION 1.0 LANGUAGES CXX)

include_directories(include)

### Project sources ###
file(GLOB_RECURSE SRCS ./src/*)

add_library(sha256 STATIC ${SRCS})

target_include_directories(sha256 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})