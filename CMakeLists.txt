project(aplqo-debug-tools)
cmake_minimum_required(VERSION 3.7)

if(CMAKE_CXX_COMPLIER MATCHES "clang" )
	add_compile_options(-stdlib=libc++)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld-8 -stdlib=libc++ -lc++fs")
elseif(CMAKE_CXX_COMPILER MATCHES "cl" )
	add_compile_options("/Zc:__cplusplus")
else()
	add_compile_options("-lstdc++fs")
endif()

option(COLOR ON)

include_directories(".")

if(COLOR)
	add_definitions(COLOR)
endif()

add_subdirectory(src)