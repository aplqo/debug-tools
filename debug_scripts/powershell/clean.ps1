[CmdletBinding()]
param (
    [Parameter()]
    [string]$path
)
Remove-Item -Path "$path\*" -Include "*.ilk", "*.obj", "*.pdb", "*.exe","*.out" -Force -Confirm:$false -Verbose