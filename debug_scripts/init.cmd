@echo off
set /p di="Enter destintion diectory "
set /p ty="Enter configure type (1: devc++ 2: MSVC/MinGW) "

if not exist %di% mkdir %di%
mkdir "%di%\.vscode"
mkdir "%di%\debug_tools"
mkdir "%di%\scripts"
mkdir "%di%\bin"
if %ty% EQU 1 copy ".\config\devc\*" "%di%\.vscode\"
if %ty% EQU 2 copy ".\config\msvc\*" "%di%\.vscode\"
copy ".\config\.clang-format" "%di%"
copy ".\debug_tools\*" "%di%\debug_tools\"
copy ".\scripts\*" "%di%\scripts\"
copy ".\bin\" "%di%\bin\"