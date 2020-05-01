include(Utility)

aux_source_directory(. src)

add_library(configs OBJECT ${src}) # MUST use object library
target_compile_definitions(configs PUBLIC -D${CMAKE_SYSTEM_NAME})
set_standard(configs)