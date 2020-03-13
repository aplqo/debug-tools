$typ = Get-Content -Path "./.config/editor"
Invoke-Expression -Command "./.dtors/deinit-$typ.ps1"
Remove-Item -Path "./.dtors/deinit-$typ.ps1" -Force -Confirm:$false