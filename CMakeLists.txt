cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(tinyDB VERSION 1.0.0)

# Compiler options
set(CMAKE_CXX_STANDARD 17)

# Modern CMake
add_executable(tinyDB)

target_include_directories(
  tinyDB PRIVATE ${PROJECT_SOURCE_DIR}/include
)

# Iterate source folder to get all source code files
file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
target_sources(tinyDB PRIVATE ${SRCS} tinyDB.cc)