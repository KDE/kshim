add_subdirectory(libkshimgen)
add_subdirectory(kshim)

if (NOT KSHIM_DISABLE_SHIMGEN)
    add_subdirectory(kshimgen)
endif()
