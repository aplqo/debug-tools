project(aplqo-debug-tools)
cmake_minimum_required(VERSION 3.7)

if(CMAKE_CXX_COMPLIER MATCHES "clang" )
	add_compile_options(-stdlib=libc++)
	link_libraries(pthread)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld-8 -stdlib=libc++")
	link_libraries(c++fs)
elseif(CMAKE_CXX_COMPILER MATCHES "cl" )
	add_compile_options("/Zc:__cplusplus")
else()
	add_compile_options("-std=c++17" "-lstdc++fs")
	link_libraries(pthread)
	link_libraries(stdc++fs)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/CMakeFiles")
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
	message("Color mode enabled")
	add_compile_definitions(COLOR)
endif()

include_directories(".")
include_directories(${CMAKE_BINARY_DIR})

add_subdirectory(src)
add_subdirectory(debug_config)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/debug_tools/ DESTINATION debug_tools/)
install(FILES ${CMAKE_BINARY_DIR}/include/define.h DESTINATION debug_tools/)
install(FILES ./include/logfile.h DESTINATION debug_tools/)
install(FILES ./LICENSE DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/debug_scripts/ DESTINATION scripts/ PATTERN "*" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_VENDOR "aplqo")
if(color)
	set(CPACK_PACKAGE_FILE_NAME aplqo-debug-tools-${CMAKE_SYSTEM_NAME}-${ver}-color)
else()
	set(CPACK_PACKAGE_FILE_NAME aplqo-debug-tools-${CMAKE_SYSTEM_NAME}-${ver}-nocolor)
endif()
include(CPack)
