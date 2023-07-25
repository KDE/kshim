
function(add_shim target file)
    if(NOT MSVC)
        if (APPLE)
            add_custom_command(TARGET ${target} POST_BUILD COMMAND strip -x -S "$<TARGET_FILE:${target}>")
        else()
            add_custom_command(TARGET ${target} POST_BUILD COMMAND strip -s "$<TARGET_FILE:${target}>")
        endif()
    endif()
    add_custom_command(OUTPUT ${file} DEPENDS ${target})
    cmrc_add_resources(kshim_embedded_resource ${file} WHENCE ${PROJECT_BINARY_DIR})
endfunction()

set(KSHIM_SRC kshimmain.cpp)

if (WIN32)
    list(APPEND KSHIM_SRC main_win.cpp)
else()
    list(APPEND KSHIM_SRC main_unix.cpp)
endif()

cmrc_add_resource_library(kshim_embedded_resource
    TYPE OBJECT
    NAMESPACE KShimEmbeddeResource
    WHENCE ${PROJECT_BINARY_DIR})

add_executable(kshim ${KSHIM_SRC})
target_link_libraries(kshim libkshimgen)
add_shim(kshim ${PROJECT_BINARY_DIR}/bin/kshim${CMAKE_EXECUTABLE_SUFFIX})



if (WIN32)
    add_executable(kshimgui WIN32 ${KSHIM_SRC})
    target_link_libraries(kshimgui libkshimgen)
    add_shim(kshimgui ${PROJECT_BINARY_DIR}/bin/kshimgui${CMAKE_EXECUTABLE_SUFFIX})
endif()
