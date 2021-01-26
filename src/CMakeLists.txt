aux_source_directory(./testcase-lib testsrc)
aux_source_directory(./testtools-lib toolsrc)

find_package(fmt CONFIG REQUIRED)
link_libraries(fmt::fmt-header-only)

set(Boost_USE_STATIC_LIBS ON)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	find_package(Boost REQUIRED COMPONENTS stacktrace_backtrace thread)
else()
	find_package(Boost REQUIRED COMPONENTS stacktrace_windbg thread)
endif()

add_subdirectory(system-lib)
add_subdirectory(judge-lib)

add_library(testtools-lib STATIC ${toolsrc})
target_link_libraries(testtools-lib PUBLIC system_full)
target_link_libraries(testtools-lib PRIVATE staticLibConfig)

add_library(testcase-tra STATIC ${testsrc})
target_compile_definitions(testcase-tra PUBLIC Tradition)
target_link_libraries(testcase-tra PUBLIC testtools-lib PRIVATE staticLibConfig)
add_library(testcase-int STATIC ${testsrc})
target_compile_definitions(testcase-int PUBLIC Interact)
target_link_libraries(testcase-int PUBLIC testtools-lib PRIVATE staticLibConfig)

function(add_target target_name src extra)
	add_executable(${target_name}-tra ${src})
	add_executable(${target_name}-int ${src}) 
	target_link_libraries(${target_name}-tra testcase-tra ${extra} exeConfig)
	target_link_libraries(${target_name}-int testcase-int ${extra} exeConfig)
	install(TARGETS ${target_name}-int DESTINATION bin/)
	install(TARGETS ${target_name}-tra DESTINATION bin/)
endfunction()

add_target(single singletest.cpp "")
add_target(group grouptest.cpp "")
add_target(random randomtest.cpp Boost::thread)

add_executable(diff autodiff.cpp)
target_link_libraries(diff testtools-lib exeConfig)
install(TARGETS diff DESTINATION bin/)

add_subdirectory(init-lib)
add_executable(init init.cpp)
target_link_libraries(init configs system_full exeConfig)
install(TARGETS init DESTINATION .)
