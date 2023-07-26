set(CMAKE_INCLUDE_CURRENT_DIR ON)
configure_file(test_config.h.in test_config.h @ONLY )

add_executable(dump_args helper/dump_args.cpp)
target_include_directories(dump_args PRIVATE ${PROJECT_SOURCE_DIR}/src/3dparty/)

add_executable(binaryName_test binaryName_test.cpp)
target_link_libraries(binaryName_test libkshimgen)
add_test(TestBinaryName ${CMAKE_BINARY_DIR}/bin/binaryName_test)

add_executable(createShim_test createShim_test.cpp)
target_include_directories(createShim_test PRIVATE ${PROJECT_SOURCE_DIR}/src/3dparty/)
target_link_libraries(createShim_test libkshimgen)
add_test(TestCreateShim ${CMAKE_BINARY_DIR}/bin/createShim_test)
add_dependencies(createShim_test dump_args)
