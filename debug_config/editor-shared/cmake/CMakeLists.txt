project(source CXX)
cmake_minimum_required(VERSION 3.12)

aux_source_directory(. src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_BUILD_TYPE MATCHES "Release" AND NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        link_libraries(pthread)
endif()

foreach(i IN LISTS src)
        get_filename_component(f ${i} NAME_WE)
        add_executable(${f} ${i})

        if(CMAKE_BUILD_TYPE MATCHES "Release" )
                if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
                        target_compile_options(${f} PUBLIC "-Wall" "-Wextra" "-Weffc++" "-Wdisabled-optimization")
                elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
                        target_compile_options(${f} PUBLIC "/Zc:__cplusplus" "/W4")
                else()
                        target_compile_options(${f} PUBLIC "-Wall" "-Wextra" "-Weffc++" "-Wdisabled-optimization")
                endif()
                target_compile_definitions(${f} PUBLIC -DAPTEST)
        else()
                target_compile_definitions(${f} PUBLIC -DAPDEBUG)
        endif()

        target_compile_features(${f} PUBLIC "cxx_std_11")
endforeach()