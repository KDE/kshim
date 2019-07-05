add_executable(kshimgen)
if (WIN32)
    target_sources(kshimgen PRIVATE main_win.cpp)
else()
    target_sources(kshimgen PRIVATE main_unix.cpp)
endif()
target_link_libraries(kshimgen libkshimgen)

install(TARGETS kshimgen RUNTIME DESTINATION bin)
