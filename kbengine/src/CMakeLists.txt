list(APPEND kbengine_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/kbengine-test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/kbengine/kbengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/kbengine/vnlexi.cpp
)


add_library(kbengine ${kbengine_SOURCES})