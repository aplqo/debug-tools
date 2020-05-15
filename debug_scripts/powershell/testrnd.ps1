[CmdletBinding()]
param(
    [String]$pro,
    [String]$generator,
    [String]$genarg,
    [String]$tmpdir,
    [String]$time
)
./bin/random $pro -tmpdir "$tmpdir" -generator "$generator" -genargs $genarg -times $time -stop-on-error -test "bin\diff" -testargs 14 -quiet -limit 0 -files 1 [input] -diff [differ] -test fc -testargs 2 [output] [answer]