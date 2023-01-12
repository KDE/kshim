set(CMAKE_INCLUDE_CURRENT_DIR ON)
configure_file(test_config.h.in test_config.h @ONLY )

add_executable(path_test path_test.cpp)
target_link_libraries(path_test libkshimgen)
add_test(TestPathLib ${CMAKE_BINARY_DIR}/bin/path_test)

add_executable(binaryName_test binaryName_test.cpp)
target_link_libraries(binaryName_test libkshimgen)
add_test(TestBinaryNameLib ${CMAKE_BINARY_DIR}/bin/binaryName_test)
