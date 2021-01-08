aux_source_directory(./testcase-lib libsrc)
aux_source_directory(./exceptions libex)

include(Utility)

add_library(testcase-lib STATIC ${libsrc})
set_standard(testcase-lib)

function(add_target target_name src)
	add_executable(${target_name} ${src})
	set_standard(${target_name})
	target_link_libraries(${target_name} debug-lib)
	target_link_libraries(${target_name} exceptions)
	install(TARGETS ${target_name} DESTINATION bin/)
endfunction()

add_target(single singletest.cpp)
add_target(group grouptest.cpp)
add_target(diff autodiff.cpp)
add_target(random randomtest.cpp)

add_subdirectory(init-lib)
add_executable(init init.cpp)
set_standard(init)
target_link_libraries(init debug-lib)
target_link_libraries(init configs)
install(TARGETS init DESTINATION .)
