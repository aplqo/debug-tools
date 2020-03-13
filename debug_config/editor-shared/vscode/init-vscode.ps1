param(
    [String]$path,
    [String]$compiler,
    [String]$extra
)

. ./config/compiler-init.ps1

function def([String]$path, [String]$conf) {
    mkdir -Path "$path/.vscode"
    Copy-Item -Path "$conf/*" -Destination "$path/.vscode"
}

init $Function:def "$path" "vscode" "$compiler" "$extra"