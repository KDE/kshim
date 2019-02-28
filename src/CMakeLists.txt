
add_subdirectory(3dparty)

add_executable(kshimgen main.cpp kshimdata.cpp kshim.cpp)
if (WIN32)
    target_sources(kshimgen PRIVATE kshim_win.cpp)
else()
    target_sources(kshimgen PRIVATE kshim_unix.cpp)
endif()

target_link_libraries(kshimgen PRIVATE nlohmann_json::nlohmann_json args)
target_compile_definitions(kshimgen PRIVATE _CRT_SECURE_NO_WARNINGS)

install(TARGETS kshimgen RUNTIME DESTINATION bin)
