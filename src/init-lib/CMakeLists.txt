include(Utility)

add_library(configs OBJECT "vscode.cpp" "init-util.cpp") # MUST use object library
target_compile_definitions(configs PUBLIC -D${CMAKE_SYSTEM_NAME})
set_standard(configs)