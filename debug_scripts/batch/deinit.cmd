cd ../
cmd /c .\scripts\clean.cmd
set /p di=< .config\src

start %di%\init deinit %cd%