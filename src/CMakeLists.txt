
add_subdirectory(3dparty)

add_executable(kshimgen main.cpp kshimdata.cpp kshim.cpp)
target_link_libraries(kshimgen PRIVATE nlohmann_json::nlohmann_json args)
target_compile_definitions(kshimgen PRIVATE _CRT_SECURE_NO_WARNINGS)

install(TARGETS kshimgen RUNTIME DESTINATION bin)
