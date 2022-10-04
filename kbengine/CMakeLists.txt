cmake_minimum_required(VERSION 3.0.0)
project(kbengine VERSION 0.1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include(CTest)
enable_testing()

add_subdirectory(src)
# list(APPEND SOURCES src
#     # ${CMAKE_CURRENT_LIST_DIR}/kbengine-test.cpp
#     # ${CMAKE_CURRENT_LIST_DIR}/kbengine/kbengine.cpp
#     # ${CMAKE_CURRENT_LIST_DIR}/kbengine/vnlexi.cpp
# )
# list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/src")


# # include(src/CMakeLists.txt)

add_executable(kbengine_test src/kbengine-test.cpp)
target_link_libraries(kbengine_test PUBLIC kbengine)

# # add the binary tree to the search path for include files
# # so that we will find TutorialConfig.h
# target_include_directories(kbengine PUBLIC
#                            "${PROJECT_BINARY_DIR}"
#                            ${EXTRA_INCLUDES}
#                            )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
