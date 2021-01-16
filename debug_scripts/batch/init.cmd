cmd /c .\scripts\clean.cmd
set /p di=< .config\src

start %di%\init %1 %cd%