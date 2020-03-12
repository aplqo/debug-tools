function deinit([System.Action]$def, [String]$editor) {
    $typ = Get-Content -Path "./config/$editor"
    if (Test-Path -Path "./.dtors/deinit-$typ.ps1") {
        Invoke-Expression -Command "./.dtors/deinit-$typ.ps1"
        Remove-Item -Path "./.dtors/deinit-$typ.ps1" -Confirm:$false
    }
    else {
        $def.Invoke()
    }
    Remove-Item -Path ./.dtors/compiler-deinit.ps1 -Confirm:$false
}