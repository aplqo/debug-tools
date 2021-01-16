[CmdletBinding()]
param(
    [String]$pro,
    [String]$data,
    [String]$regex = '^(.*)\.[^\.]*', # match path without extension
    [String]$inreg = '.*\.in$',
    [String]$ansreg = '.*\.ans$'
)

& ./bin/group-$env:type -program "$pro"  -test-regex [ "$regex" -em 1 ';' ] -indir "$data"  -in-regex  [ "$inreg" -mm  ';' ]  -ansdir "$data" -ans-regex [ "$ansreg" -mm ';' ] -test "fc.exe" -test-args [ "{output}" "{answer}" ] -autodiff [ -diff '{input}.diff' -files [ '{output}' '{answer}' ] ] ';' 