cmake_minimum_required(VERSION 3.5)

project(KShim LANGUAGES CXX VERSION 0.3.0)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
include(cmakerc/CMakeRC)

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
    set (CMAKE_CXX_STANDARD 14)
endif()

#link runtime static
if(MSVC)
    foreach(_bt DEBUG RELEASE RELWITHDEBINFO)
            string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_${_bt} ${CMAKE_CXX_FLAGS_${_bt}})
    endforeach(_bt DEBUG RELEASE RELWITHDEBINFO)
elseif(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(CMAKE_EXE_LINKER_FLAGS ${MAKE_EXE_LINKER_FLAGS} -static )
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
