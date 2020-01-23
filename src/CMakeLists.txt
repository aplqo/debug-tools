aux_source_directory(./debug-lib libsrc)
aux_source_directory(./exceptions libex)

add_library(debug-lib STATIC ${libsrc})
add_library(exceptions STATIC ${libex})
target_compile_features(debug-lib PUBLIC "cxx_std_17")

add_executable(single singletest.cpp)
target_compile_features(single PUBLIC "cxx_std_17")
target_link_libraries(single debug-lib)
target_link_libraries(single exceptions)

install(TARGETS single DESTINATION bin/)