set /p di=< .config\src
set /p typ=< .config\typ
set cur=%cd%

del /s /q .clang-format
rmdir /s /q .vscode
rmdir /s /q debug_tools
rmdir /s /q bin
rmdir /s /q .config

pushd
cd %di%
init.cmd %cur% %typ%
popd