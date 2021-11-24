cmake_minimum_required(VERSION 3.0.0)
project(Proj VERSION 0.1.0)

### CMake config ###
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  add_compile_definitions(SRPF__DEBUG)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
  message(STATUS "GCC detected, adding compile flags")
  set(CMAKE_CXX_FLAGS "-Wall -Wextra")
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
else(CMAKE_COMPILER_IS_GNUCXX)
  message(STATUS "MSVC detected, adding compile flags")
  set(CMAKE_CXX_FLAGS "/EHsc")
  set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi")
  set(CMAKE_CXX_FLAGS_RELEASE "/O2")
endif(CMAKE_COMPILER_IS_GNUCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../")

### Project headers ###
include_directories(include)

### Project Dependencies ###
add_subdirectory(./lib/sha256)
include_directories(./lib/sha256)

set(BUILD_SHARED_LIBS OFF)
find_package(SFML 2.5.1 COMPONENTS network REQUIRED)

### Project sources ###
file(GLOB_RECURSE SRCS ./src/SFML-2.5.1/cmake*)
file(GLOB_RECURSE HEADERS ./include/*)

add_executable(Proj ${HEADERS} ${SRCS})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

target_link_libraries(
    Proj
    sha256
    sfml-network
)
