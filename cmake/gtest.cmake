# Introduce google test
include(FetchContent)
FetchContent_Declare(gtest
    GIT_REPOSITORY  https://github.com/google/googletest.git
    GIT_TAG         release-1.10.0
    SOURCE_DIR      ${PROJECT_SOURCE_DIR}/3rd/gtest
)
# New Visual Studio projects link the C runtimes dynamically 
# But GoogleTest links them statically
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(gtest)
