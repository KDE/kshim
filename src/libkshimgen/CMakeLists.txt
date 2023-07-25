configure_file(config.h.in config.h @ONLY)

add_library(libkshimgen STATIC kshimdata.cpp kshim.cpp)
if (WIN32)
    target_sources(libkshimgen PRIVATE kshim_win.cpp)
else()
    target_sources(libkshimgen PRIVATE kshim_unix.cpp)
endif()
target_include_directories(libkshimgen PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

if(WIN32)
    target_compile_definitions(libkshimgen PUBLIC _CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN UNICODE _UNICODE)
endif()

target_link_libraries(libkshimgen PUBLIC ${FILESYSTEM_LIBS})
