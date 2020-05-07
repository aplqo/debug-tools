[CmdletBinding()]
param (
    [String]$pre,
    [Int32]$times,
    [Bool]$cpp = $false
)

if ($cpp) {
    New-Item -Path "$pre.cpp"
}
for ($i = 0; $i -lt $times; $i++) {
    New-Item -Path "$pre$i.in"
    New-Item -Path "$pre$i.ans"
    Start-Process -FilePath "$env:edit" -ArgumentList "$pre$i.in" -Wait
    Start-Process -FilePath "$env:edit" -ArgumentList "$pre$i.ans" -Wait
}