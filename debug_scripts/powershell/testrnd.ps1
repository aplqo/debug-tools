[CmdletBinding()]
param(
    [String]$pro,
    [String]$generator,
    [String]$genarg,
    [String]$tmpdir,
    [String]$time
)
& ./bin/random-$env:type -program $pro -tmpdir "$tmpdir" -generator "$generator" -gen-args $genarg -times $time -stop-on-error  -test fc -test-args [ "{output}" "{answer}" ] 