add_executable(kshimgen WIN32 kshimdata.cpp kshim.cpp)
if (WIN32)
    target_sources(kshimgen PRIVATE kshim_win.cpp)
else()
    target_sources(kshimgen PRIVATE kshim_unix.cpp)
    target_link_libraries(kshimgen PRIVATE stdc++fs)
endif()

target_compile_definitions(kshimgen PRIVATE _CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN)

install(TARGETS kshimgen RUNTIME DESTINATION bin)
