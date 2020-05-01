Set-Location ..
& ./scripts/clean.ps1

$rt = Get-Content -Path "./.config/src"
Start-Process -FilePath "$rt/init" -ArgumentList "deinit", "$(Get-Location)"