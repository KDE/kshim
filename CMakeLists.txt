cmake_minimum_required(VERSION 3.5)

project(KShim VERSION 0.1)


set(FILESYSTEM_LIBS)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(FILESYSTEM_LIBS stdc++fs)
endif()

try_compile(KSHIM_HAS_FILESYSTEM ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/test_filesystem.cpp
    CXX_STANDARD 17
    LINK_LIBRARIES ${FILESYSTEM_LIBS}
    OUTPUT_VARIABLE KSHIM_HAS_FILESYSTEM_LOG
    )

if (KSHIM_HAS_FILESYSTEM)
    message(STATUS "Using std::filesystem")
    set (CMAKE_CXX_STANDARD 17)
else()
    message(STATUS "Using minimal replacement for std::filesystem")
    #message(STATUS "${KSHIM_HAS_FILESYSTEM_LOG}")
    set (CMAKE_CXX_STANDARD 11)
endif()


add_subdirectory(src)
add_subdirectory(tests)
