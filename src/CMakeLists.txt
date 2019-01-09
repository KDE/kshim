
add_executable(kshimgen main.cpp kshimdata.cpp kshim.cpp)

install(TARGETS kshimgen RUNTIME DESTINATION bin)
