cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0056 NEW)

project(KShim LANGUAGES CXX VERSION 0.6.0)
set (CMAKE_CXX_STANDARD_REQUIRED 17)
set (CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
include(cmakerc/CMakeRC)
include(CTest)

set(FILESYSTEM_LIBS)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(FILESYSTEM_LIBS stdc++fs)
endif()

#link runtime static
if(MSVC)
    foreach(_bt RELEASE RELWITHDEBINFO)
            string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_${_bt} ${CMAKE_CXX_FLAGS_${_bt}})
    endforeach()
elseif(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(CMAKE_EXE_LINKER_FLAGS_OLD ${CMAKE_EXE_LINKER_FLAGS})
    set(CMAKE_EXE_LINKER_FLAGS ${MAKE_EXE_LINKER_FLAGS} -static )
    try_compile(HAS_STATIC_RUNTIME ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/test_static_runtime.cpp)
    if (NOT HAS_STATIC_RUNTIME)
        message(STATUS "Failed to compile using static runtime, using dynamic runtime")
        set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS_OLD})
    endif()
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

# enable more warnings
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4" )
endif()

add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
