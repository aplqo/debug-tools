set prefix=.\config\vscode
mkdir "%1\.vscode"
echo "%2" > "%1\.config\vscode"
if exist "%preifx%\%2\%2-init.cmd" (
    %preifx%\%2\%2-init.cmd "%1"
) else (
    copy "%prefix%\%2\*" "%1\.vscode\"
)