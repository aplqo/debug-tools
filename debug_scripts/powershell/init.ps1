[CmdletBinding()]
param (
	[String]$operate
)
$rt = Get-Content -Path "./.config/src"
Start-Process -FilePath "$rt/init" -ArgumentList "$operate", "$(Get-Location)"