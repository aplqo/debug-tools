aux_source_directory(./debug-lib libsrc)
aux_source_directory(./exceptions libex)

add_library(debug-lib STATIC ${libsrc})
add_library(exceptions STATIC ${libex})
target_compile_features(debug-lib PUBLIC "cxx_std_17")

add_executable(single singletest.cpp)
target_compile_features(single PUBLIC "cxx_std_17")
target_link_libraries(single debug-lib)
target_link_libraries(single exceptions)

add_executable(group grouptest.cpp)
target_compile_features(group PUBLIC "cxx_std_17")
target_link_libraries(group debug-lib)
target_link_libraries(group exceptions)

add_executable(diff autodiff.cpp)
target_link_libraries(diff debug-lib)
target_compile_features(diff PUBLIC "cxx_std_17")


install(TARGETS single DESTINATION bin/)
install(TARGETS group DESTINATION bin/)
install(TARGETS diff DESTINATION bin/)