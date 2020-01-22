cd ../
cmd /c .\scripts\clean.cmd
del /s /q .clang-format
rmdir /s /q .vscode
rmdir /s /q debug_tools
rmdir /s /q bin
rmdir /s /q scripts