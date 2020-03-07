set prefix=.\config\vscode
mkdir "%1\.vscode"
if exist "%preifx%\%2\%2-init.cmd" (
    %preifx%\%2\%2-init.cmd "%1"
) else (
    copy "%prefix%\%2\*" "%1\.vscode\"
)
echo "%2" > "%1\.config\vscode"
if exist "%prefix%\%2\%2-deinit.cmd" (
    copy "%prefix%\%2-deinit.cmd" "%1\.dtors"
    del /s /q "%1\scripts\%2-deinit.cmd"
)