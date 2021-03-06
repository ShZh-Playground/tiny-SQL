# Unit test
enable_testing()

# Introduce google test
set(CMAKE_MODULE_PATH "${CMAKE_DIR};${CMAKE_MODULE_PATH}")
include(gtest)
include(GoogleTest)   # Introduce custom command
if (WIN32 AND (NOT CYGWIN) AND (NOT MINGW))
	set(gtest_disable_pthreads ON)
endif()

# Automatically add file to unit test set
macro(tinyDB_add_test name)
	add_executable(test_${name}
		${TEST_DIR}/test_${name}.cc
	)
	# Compiling order will be implicitly specified
	target_link_libraries(test_${name} gtest tinyDB)
	gtest_discover_tests(test_${name})
	# No need to use ctest
endmacro()

# List of unit test files
set(UNITTEST_SOURCES
	sql_statement
	meta_command
)

foreach(test_file ${UNITTEST_SOURCES})
	tinyDB_add_test(${test_file})
endforeach()
