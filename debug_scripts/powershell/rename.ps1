[CmdletBinding()]
param (
    [Parameter()]
    [string]$path
)
foreach ($i in (Get-ChildItem -Path "$path\*" -Include "*.out")) {
    Rename-Item -Path $i.ToString() -NewName "$($i.BaseName).ans" -Verbose
}