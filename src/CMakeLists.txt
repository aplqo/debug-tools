aux_source_directory(. src)
aux_source_directory(./exceptions ex)
list(FILTER src EXCLUDE REGEX "singletest.cpp")

add_library(debug-lib STATIC ${src})
add_library(exceptions STATIC ${ex})
target_compile_features(debug-lib PUBLIC "cxx_std_17")

add_executable(single singletest.cpp)
target_compile_features(single PUBLIC "cxx_std_17")
target_link_libraries(single debug-lib)
target_link_libraries(single exceptions)

install(TARGETS single DESTINATION bin/)