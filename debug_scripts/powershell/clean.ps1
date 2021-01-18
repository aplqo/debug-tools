[CmdletBinding()]
param (
    [Parameter()]
    [string]$path
)
Remove-Item -Path "$path\*" -Include "*.ilk", "*.obj", "*.pdb", "*.exe","*.out","*.diff" -Force -Confirm:$false -Verbose