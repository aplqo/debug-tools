aux_source_directory(. common)
aux_source_directory(./${CMAKE_SYSTEM_NAME} sys)

add_library(system_full STATIC ${common} ${sys}) # full system library for testcase-lib

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_library(system_judge STATIC 
        Linux/process.cpp Linux/shm.cpp Linux/protectPage.cpp Linux/usage.cpp roundToPage.cpp timeUsage.cpp
    ) # min library for judge
    target_link_libraries(system_full PUBLIC rt pthread)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_library(system_judge STATIC
        Windows/process.cpp Windows/protectPage.cpp Windows/shm.cpp Windows/usage.cpp roundToPage.cpp timeUsage.cpp
    ) # min library for judge/init
endif()
    
target_link_libraries(system_full PRIVATE staticLibConfig yaml-cpp)
target_link_libraries(system_judge PRIVATE staticLibConfig)

set_property(TARGET system_judge PROPERTY INTERPROCEDURAL_OPTIMIZATION ${outputLTO})
install(TARGETS system_judge DESTINATION lib/)