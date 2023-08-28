SET(target_test test_njson)

ADD_EXECUTABLE(${target_test} test_njson.cpp)
TARGET_LINK_LIBRARIES(${target_test} GTest::gtest_main njson)
ADD_DEPENDENCIES(${target_test} njson)
FILE(COPY test.json DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

INCLUDE(GoogleTest)
gtest_discover_tests(${target_test})
