cmake_minimum_required(VERSION 3.9)
project(debug-tools)

if(CMAKE_CXX_COMPILER MATCHES "clang" )
	add_compile_options("-stdlib=libc++")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld -stdlib=libc++")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
        	link_libraries(c++fs)
	endif()
elseif(CMAKE_CXX_COMPILER MATCHES "cl" )
	add_compile_options("/Zc:__cplusplus" "/std:c++latest" "/utf-8")
else()
	link_libraries(stdc++fs)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
	message(STATUS "LTO enabled")
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION true)
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(branch "")
set(hash "")
# get git information
find_package(Git QUIET)
if(GIT_FOUND)
	execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%H
		OUTPUT_VARIABLE hash
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
	execute_process(COMMAND ${GIT_EXECUTABLE} symbolic-ref --short -q HEAD
		OUTPUT_VARIABLE branch
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
	if("${branch}" STREQUAL "")
		message(STATUS "Using detached head")
		execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --exact-match HEAD
			OUTPUT_VARIABLE branch
			OUTPUT_STRIP_TRAILING_WHITESPACE
			ERROR_QUIET
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
	endif()
endif()

set(build "unknown")
#get host information
execute_process(COMMAND hostname
	OUTPUT_VARIABLE build
	OUTPUT_STRIP_TRAILING_WHITESPACE
	ERROR_QUIET
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/include/define.h.in
	${CMAKE_BINARY_DIR}/include/define.h
	@ONLY
)

option(color "Whether enable color output" on)
if(color)
	message(STATUS "Color output enabled")
	add_compile_definitions(COLOR)
endif()
option(static_link "Whether static link libraried")

add_library(exeConfig INTERFACE)
add_library(staticLibConfig ALIAS exeConfig)
if(static_link)
	message(STATUS "Static link enabled")
	add_compile_definitions(STATIC_LINK)
	if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		target_compile_options(exeConfig INTERFACE "-static")
		target_link_options(exeConfig INTERFACE "-static")
	else()
		target_compile_options(exeConfig INTERFACE "/MT")
	endif()
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(cgroupVersion 2 CACHE STRING "Which version output_required_files cgroup is used)")
	add_compile_definitions(cgroupVersion=${cgroupVersion})
endif()

include_directories(".")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/${CMAKE_SYSTEM_NAME}")
include_directories(${CMAKE_BINARY_DIR})

add_subdirectory(src)
add_subdirectory(debug_config)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/debug_tools/ DESTINATION debug_tools/)
install(FILES ./include/logfile.h DESTINATION debug_tools/)
install(FILES ./LICENSE DESTINATION .)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/debug_scripts/ DESTINATION scripts/ PATTERN "*" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_VENDOR "aplqo")
set(CPACK_PACKAGE_FILE_NAME ${CMAKE_SYSTEM_NAME}-${ver})
if (CMAKE_BUILD_TYPE STREQUAL "Release")
	message(STATUS "Output file stripped")
	set(CPACK_STRIP_FILES TRUE)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME}-cg${cgroupVersion})
endif()
if(static_link)
	set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME}-static)
else()
	set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME}-shared)
endif()
include(CPack)
