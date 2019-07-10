set(KSHIM_SRC kshimmain.cpp)

if (WIN32)
    list(APPEND KSHIM_SRC main_win.cpp)
else()
    list(APPEND KSHIM_SRC main_unix.cpp)
endif()

add_executable(kshim ${KSHIM_SRC})
target_link_libraries(kshim libkshimgen)
# TODO: get target name
add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/bin/kshim${CMAKE_EXECUTABLE_SUFFIX} DEPENDS kshim)

cmrc_add_resource_library(kshim_embedded_resource
    ${PROJECT_BINARY_DIR}/bin/kshim${CMAKE_EXECUTABLE_SUFFIX}
    NAMESPACE KShimEmbeddeResource
    WHENCE ${PROJECT_BINARY_DIR})

if (WIN32)
    add_executable(kshimgui WIN32 ${KSHIM_SRC})
    target_link_libraries(kshimgui libkshimgen)
    # TODO: get target name
    add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/bin/kshimgui${CMAKE_EXECUTABLE_SUFFIX} DEPENDS kshimgui)
    cmrc_add_resources(kshim_embedded_resource ${PROJECT_BINARY_DIR}/bin/kshimgui${CMAKE_EXECUTABLE_SUFFIX} WHENCE ${PROJECT_BINARY_DIR})
endif()






