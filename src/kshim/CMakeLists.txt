# console applications as window applications doesnt work
#add_executable(kshimgen WIN32)
if (WIN32)
    set(KSHIM_SRC main_win.cpp)
else()
    set(KSHIM_SRC main_unix.cpp)
endif()

add_executable(kshim ${KSHIM_SRC})
target_link_libraries(kshim libkshimgen)
install(TARGETS kshim RUNTIME DESTINATION bin)

if (WIN32)
    add_executable(kshimgui WIN32 ${KSHIM_SRC})
    target_link_libraries(kshimgui libkshimgen)
    install(TARGETS kshimgui RUNTIME DESTINATION bin)
endif()


