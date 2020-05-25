[CmdletBinding()]
param(
    [String]$pro,
    [String]$data,
    [String]$regex = "^",
    [String]$inreg = '\.in$',
    [String]$ansreg = '\.ans$'
)

./bin/group "$pro" -test-regex "$regex" -indir "$data" -in-regex "$inreg" -ansdir "$data" -ans-regex "$ansreg" -test "powershell .\scripts\test.ps1" -testargs [ "<input>" "<output>" "<answer>" ]