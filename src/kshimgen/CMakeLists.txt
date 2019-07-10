add_executable(kshimgen kshimgen.cpp)
if (WIN32)
    target_sources(kshimgen PRIVATE main_win.cpp)
else()
    target_sources(kshimgen PRIVATE main_unix.cpp)
endif()
target_link_libraries(kshimgen PRIVATE libkshimgen kshim_embedded_resource)

install(TARGETS kshimgen RUNTIME DESTINATION bin)
