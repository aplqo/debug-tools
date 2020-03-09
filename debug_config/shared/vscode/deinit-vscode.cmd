set /p typ=< .\config\vscode
if exist ".\dtors\deinit-%typ%.cmd" (
    .\dtors\deinit-%typ%.cmd
    del /s /q .\dtors\deinit-%typ%.cmd
)
rmdir /s /q .vscode