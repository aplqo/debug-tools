install(DIRECTORY ${CMAKE_SYSTEM_NAME}/ DESTINATION config/)
install(DIRECTORY ./compiler-shared/ DESTINATION config/compiler-shared/)
install(DIRECTORY ./editor-shared/ DESTINATION config/)
install(FILES .clang-format DESTINATION config/)