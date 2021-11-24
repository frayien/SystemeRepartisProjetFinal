cmake_minimum_required(VERSION 3.0.0)
project(sha256 VERSION 1.0 LANGUAGES CXX)

add_library(sha256 STATIC
    sha256.cpp
)

target_include_directories(sha256 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})