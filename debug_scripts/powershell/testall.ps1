[CmdletBinding()]
param(
    [String]$pro,
    [String]$data,
    [String]$regex = '^(.*)\.[^\.]*', # match path without extension
    [String]$inreg = '.*\.in$',
    [String]$ansreg = '.*\.ans$'
)

./bin/group "$pro"  -test-regex [ "$regex" -em 1 ';' ] -indir "$data"  -in-regex  [ "$inreg" -mm  ';' ]  -ansdir "$data" -ans-regex [ "$ansreg" -mm ';' ] -test "powershell .\scripts\test.ps1" -testargs [ "<input>" "<output>" "<answer>" ] ';' 