# Introduce google test
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
include(gtest)

# 以后测试文件多起来的时候可以直接遍历得到
add_executable(test_add
  ${CMAKE_CURRENT_SOURCE_DIR}/test_add.cc
)
add_dependencies(test_add
  gtest
)
target_link_libraries(test_add
  gtest
)

# Unit test
enable_testing()
# Name是测试用例的名字，test_add才是可执行文件
add_test(NAME add COMMAND test_add)