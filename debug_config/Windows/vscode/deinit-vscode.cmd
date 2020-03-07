set /p typ=< .\config\vscode
if exist ".\dtors\%typ%-deinit.cmd" (
    .\dtors\%typ%-deinit.cmd
    del /s /q .\dtors\%typ%-deinit.cmd
)
rmdir /s /q .vscode