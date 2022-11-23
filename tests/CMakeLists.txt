add_executable(path_test path_test.cpp)
target_link_libraries(path_test libkshimgen)
add_test(TestPathLib ${CMAKE_BINARY_DIR}/bin/path_test)

add_executable(binaryName_test binaryName_test.cpp)
target_compile_definitions(binaryName_test PRIVATE "-DBINARY_DIR=\"${CMAKE_BINARY_DIR}/bin\"")
target_link_libraries(binaryName_test libkshimgen)
add_test(TestBinaryNameLib ${CMAKE_BINARY_DIR}/bin/binaryName_test)
