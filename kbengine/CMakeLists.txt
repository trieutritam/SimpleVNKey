cmake_minimum_required(VERSION 3.0.0)
project(kbengine VERSION 0.1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)

# Built the kbengine-test file
add_executable(kbengine_test src/kbengine-test.cpp)
target_link_libraries(kbengine_test PUBLIC kbengine)
install(TARGETS kbengine_test DESTINATION ${CMAKE_CURRENT_LIST_DIR}/bin)
# End build kbengine-test

target_include_directories(kbengine PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# GoogleTest Primer
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)