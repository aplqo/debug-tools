param (
    [String]$pro,
    [string]$1,
    [String]$2,
    [bool]$nocpp = $false
)
if (!(Test-Path -Path "$1.cpp") -and !$nocpp) {
    New-Item -ItemType "File" -Name $1.cpp
}
for ($i = 0; $i -lt $2; $i++) {
    New-Item -ItemType "File" -Name $1$i.in
    New-Item -ItemType "File" -Name $1$i.ans
    Start-Process -FilePath "$pro" -ArgumentList "$1$i.in" -Wait
    Start-Process -FilePath "$pro" -ArgumentList "$1$i.ans" -Wait 
}