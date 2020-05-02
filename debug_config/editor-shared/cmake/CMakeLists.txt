project(source CXX)
cmake_minimum_required(VERSION 3.12)

aux_source_directory(. src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

foreach(i IN LISTS src)
	get_filename_component(f ${i} NAME_WLE)
	add_executable(${f} ${i})

	if(CMAKE_BUILD_TYPE EQUAL "Release" )
		if(CMAKE_CXX_COMPILER MATCHES "cl")
			target_compile_options(${f} "/Zc:__cplusplus")
		else()
			target_link_libraries(pthread)
		endif()
		target_compile_definitions(${f} PUBLIC -DAPTEST)
	else()
		target_compile_definitions(${f} PUBLIC -DAPDEBUG)
	endif()

	target_compile_features(${f} PUBLIC "cxx_std_11")
endforeach()
