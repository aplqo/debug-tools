aux_source_directory(. common)
aux_source_directory(./${CMAKE_SYSTEM_NAME} sys)

add_library(system_full STATIC ${common} ${sys}) # full system library for testcase-lib

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_library(system_min STATIC 
        Linux/process.cpp Linux/shm.cpp Linux/protectPage.cpp Linux/usage.cpp roundToPage.cpp
    ) # min library for judge/init
    target_link_libraries(system_full PUBLIC rt pthread)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_library(system_min STATIC
        Windows/init.cpp Windows/process.cpp Windows/protectPage.cpp Windows/shm.cpp Windows/usage.cpp roundToPage.cpp
    ) # min library for judge/init
endif()
    
target_link_libraries(system_full PRIVATE staticLibConfig)
target_link_libraries(system_min PRIVATE staticLibConfig)
install(TARGETS system_min DESTINATION lib/)