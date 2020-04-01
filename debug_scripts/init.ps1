param(
    [String]$path,
    [String]$editor,
    [String]$compiler,
    [String]$extra
)

if (!$path) {
    $path = Read-Host -Prompt "Enter file path"
}
if (!(Test-Path -Path "$path")) {
    mkdir -Path "$path" -Verbose:$false > $null
}

mkdir -Path "$path/scripts" -Verbose:$false > $null
mkdir -Path "$path/.config" -Verbose:$false > $null
mkdir "$path/.dtors" -Verbose:$false > $null

Copy-Item -Path "./bin" -Destination "$path/bin" -Recurse
Copy-Item -Path "./scripts/powershell/*" -Destination "$path/scripts"
Copy-Item -Path "./debug_tools" -Destination "$path/debug_tools" -Recurse
Copy-Item -Path "./config/.clang-format" "$path"

Out-File -InputObject "$pwd" -FilePath "$path/.config/src"
./config/config-init.ps1 "$path" "$editor" "$compiler" "$extra"