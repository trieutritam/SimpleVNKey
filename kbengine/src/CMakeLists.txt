list(APPEND kbengine_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/kbengine-test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/kbengine/kbengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/kbengine/vnlexi.cpp
)

add_library(kbengine ${kbengine_SOURCES})


# Installation
install(TARGETS kbengine DESTINATION ${CMAKE_CURRENT_LIST_DIR}/../lib)
install(DIRECTORY  ${CMAKE_CURRENT_LIST_DIR}/kbengine DESTINATION ${CMAKE_CURRENT_LIST_DIR}/../include FILES_MATCHING PATTERN "*.h")


#Test configuration

# Set additional sources
set(SOURCES test/fixture/kbengine-test.cc test/fixture/kbengine-test-telex.cc)

# Add sources which contains test cases
add_executable(test_main test/test-main.cc test/test-telex.cc test/test-vniwindows.cc ${SOURCES})
target_link_libraries(
  test_main
  GTest::gtest_main
  kbengine
)
target_include_directories(test_main PUBLIC ${CMAKE_CURRENT_LIST_DIR})

include(GoogleTest)
gtest_discover_tests(test_main)