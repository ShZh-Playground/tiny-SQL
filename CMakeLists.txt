cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(tinyDB VERSION 1.0.0)

# Compiler options
set(CMAKE_CXX_STANDARD 17)

# Build SQL binary by modern cmake
add_executable(tinyDB)
target_include_directories(
  tinyDB PRIVATE ${PROJECT_SOURCE_DIR}/include
)
# Iterate source folder to get all source code files
file(GLOB_RECURSE SRCS ${PROJECT_SOURCE_DIR}/src/*.cc)
target_sources(tinyDB PRIVATE ${SRCS} tinyDB.cc)

# Enable unit test or not
option(ENABLE_TEST "Enable test" ON)
if (ENABLE_TEST)
  add_subdirectory("test")
endif()
