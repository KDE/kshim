set(CMAKE_INCLUDE_CURRENT_DIR ON)
configure_file(test_config.h.in test_config.h @ONLY )

function(add_kshim_test TEST_NAME)
    add_executable(${TEST_NAME}_test ${TEST_NAME}_test.cpp)
    target_link_libraries(${TEST_NAME}_test libkshimgen)
    add_test(Test${TEST_NAME} ${CMAKE_BINARY_DIR}/bin/${TEST_NAME}_test)
    set_property(TARGET ${TEST_NAME}_test PROPERTY ENVIRONMENT "KSHIM_LOG=1;KSHIM_LOG_STD=1")
endfunction()

add_executable(dump_args helper/dump_args.cpp)
target_include_directories(dump_args PRIVATE ${PROJECT_SOURCE_DIR}/src/3dparty/)

add_kshim_test(binaryName)

add_kshim_test(createShim)
target_include_directories(createShim_test PRIVATE ${PROJECT_SOURCE_DIR}/src/3dparty/)
add_dependencies(createShim_test dump_args)
