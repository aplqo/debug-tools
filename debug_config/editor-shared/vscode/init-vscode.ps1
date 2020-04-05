param(
    [String]$path,
    [String]$compiler,
    [String]$extra
)

function def([String]$path, [String]$conf) {
    mkdir -Path "$path/.vscode"
    Copy-Item -Path "$conf/*" -Destination "$path/.vscode"
}

. ./config/compiler-init.ps1

init def "$path" "vscode" "$compiler" "$extra"