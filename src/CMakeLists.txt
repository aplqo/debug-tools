aux_source_directory(./debug-lib libsrc)
aux_source_directory(./exceptions libex)

add_library(debug-lib STATIC ${libsrc})
add_library(exceptions STATIC ${libex})
target_compile_features(debug-lib PUBLIC "cxx_std_17")

function(add_target target_name src)
	add_executable(${target_name} ${src})
	target_compile_features(${target_name} PUBLIC "cxx_std_17")
	target_link_libraries(${target_name} debug-lib)
	target_link_libraries(${target_name} exceptions)
	install(TARGETS ${target_name} DESTINATION bin/)
endfunction()

add_target(single singletest.cpp)
add_target(group grouptest.cpp)
add_target(diff autodiff.cpp)
add_target(random randomtest.cpp)