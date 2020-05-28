[CmdletBinding()]
param(
    [String]$pro,
    [String]$generator,
    [String]$genarg,
    [String]$tmpdir,
    [String]$time
)
./bin/random $pro -tmpdir "$tmpdir" -generator "$generator" -genargs $genarg -times $time -stop-on-error -test "bin\diff" -testargs [ -quiet -limit 0 -files "<input>" -diff "<differ>" -test fc -testargs [ "<output>" "<answer>" ] ]