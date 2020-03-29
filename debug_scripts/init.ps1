param(
    [String]$path,
    [String]$editor,
    [String]$compiler,
    [String]$extra
)

if (!(Test-Path -Path "$path")) {
    mkdir -Path "$path"
}

mkdir -Path "$path/scripts"
mkdir -Path "$1/.config"
mkdir "$path/.dtors"

Copy-Item -Path "./bin" -Destination "$path/bin" -Recurse
Copy-Item -Path "./scripts/powershell/*" -Destination "$path/scripts"
Copy-Item -Path "./debug_tools" -Destination "$path/debug_tools" -Recurse
Copy-Item -Path "./config/.clang-format" "$path"

Out-File -InputObject "$pwd" -FilePath "$path/.config/src"
./config/config-init.ps1 "$path" "$editor" "$compiler" "$extra"