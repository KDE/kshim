add_executable(path_test path_test.cpp)
target_link_libraries(path_test libkshimgen)
add_test(TestPathLib ${CMAKE_BINARY_DIR}/bin/path_test)
