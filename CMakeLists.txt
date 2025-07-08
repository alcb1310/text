cmake_minimum_required(VERSION 4.0)
project(kilo VERSION 0.1.0)
set(CMAKE_C_STANDARD 99)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic")

# Adding our source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS src/*.c)
set(PROJECT_INCLUDE "${PROJECT_SOURCE_DIR}/src")

# declaring our executable
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
