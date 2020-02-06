@echo off
if "%1" == "" ( 
    set /p di="Enter destintion diectory " 
) else (
    set di=%1
)
if "%2" == "" ( 
    set /p ty="Enter configure type (1: devc++ 2: MSVC/MinGW) " 
) else (
    set ty=%2
)

if not exist %di% mkdir %di%
mkdir "%di%\.vscode"
echo "%di%\.vscode"
mkdir "%di%\debug_tools"
mkdir "%di%\bin"
mkdir "%di%\.config"
if not exist "%di%\scripts" mkdir "%di%\scripts"
if %ty% EQU 1 copy ".\config\devc\*" "%di%\.vscode\"
if %ty% EQU 2 copy ".\config\msvc\*" "%di%\.vscode\"
copy ".\config\.clang-format" "%di%"
copy ".\debug_tools\*" "%di%\debug_tools\"
copy ".\scripts\*.cmd" "%di%\scripts\"
copy ".\bin\" "%di%\bin\"
cd > "%di%\.config\src"
echo %ty% > "%di%\.config\typ"