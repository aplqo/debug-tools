function def() {
    Remove-Item -Path ./.vscode -Recurse -Confirm:$false
}

. ./.dtors/compiler-deinit.ps1

deinit $Function:def "vscode"