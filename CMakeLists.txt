cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(tinyDB VERSION 1.0.0)

# Compiler options
set(CMAKE_CXX_STANDARD 17)
# Disable MSVC unnessary warnings
if(MSVC)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Project skeleton
set(SOURCE_DIR 	${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(BINARY_DIR  ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_DIR   ${PROJECT_SOURCE_DIR}/cmake)
set(TEST_DIR    ${PROJECT_SOURCE_DIR}/test)

# Build dynamic library for unit test
add_library(tinyDB STATIC)
target_include_directories(tinyDB
	PUBLIC ${INCLUDE_DIR}
)
file(GLOB_RECURSE SRCS 	${SOURCE_DIR}/*.cc)
target_sources(tinyDB PUBLIC ${SRCS})
if (MSVC)
	target_compile_definitions(tinyDB PRIVATE DLL_EXPORT)
endif()

# Build SQL binary by modern cmake
add_executable(tinyDBExe)
target_sources(tinyDBExe PRIVATE tinyDB.cc)
target_link_libraries(tinyDBExe tinyDB)

message("Current build type is " ${CMAKE_BUILD_TYPE})
if (${CMAKE_BUILD_TYPE} MATCHES Debug)
	# Enable unit test
	add_subdirectory("test")
else()
	# Make a copy to ./bin when building
	set_target_properties(tinyDBExe
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR}
	)
endif()

# Clean the temp dir
# It will report error but still works
add_custom_command(
	OUTPUT rm_target
	COMMAND ${CMAKE_COMMAND} -E 
		remove_directory ${BINARY_DIR} ${PROJECT_SOURCE_DIR}/build
)
add_custom_target(
	rm
	DEPENDS rm_target
)
