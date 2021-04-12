# 在config的阶段（生成build system）的时候引入
# 另外一个ExternalProject是在编译的时候引入
include(FetchContent)
FetchContent_Declare(gtest
  GIT_REPOSITORY  https://github.com/google/googletest.git
  GIT_TAG         release-1.10.0
  SOURCE_DIR      ${PROJECT_SOURCE_DIR}/3rd/gtest
)
# configure build of googletest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(gtest)
