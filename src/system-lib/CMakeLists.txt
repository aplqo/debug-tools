aux_source_directory(. common)
aux_source_directory(./${CMAKE_SYSTEM_NAME} sys)

add_library(system_full STATIC ${common} ${sys}) # full system library for testcase-lib

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_library(system_min STATIC 
        Linux/process.cpp Linux/shm.cpp Linux/protectPage.cpp Linux/usage.cpp roundToPage.cpp
    ) # min library for judge/init

    if (static_link)
        target_link_libraries(system_full PRIVATE rt staticLibConfig)
        target_compile_options(system_full PUBLIC "-pthread")
        target_link_options(system_full PUBLIC "-Wl,--whole-archive" "-lpthread" "-Wl,--no-whole-archive")
    else()
        target_link_libraries(system_full PUBLIC rt pthread)
        target_link_libraries(system_min PRIVATE staticLibConfig)
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_library(system_min STATIC
        Windows/init.cpp Windows/process.cpp Windows/protectPage.cpp Windows/shm.cpp Windows/usage.cpp roundToPage.cpp
    ) # min library for judge/init
    target_link_libraries(system_full PRIVATE staticLibConfig)
endif()

target_link_libraries(system_min PRIVATE staticLibConfig)
install(TARGETS system_min DESTINATION lib/)